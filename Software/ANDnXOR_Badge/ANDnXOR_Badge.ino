#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <Arduino.h>
#include <libmaple/gpio.h>
#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <libmaple/usb.h>
#include <RFM69-ANDnXOR.h>
#include <RTClock.h>
#include <SPI.h>
#include <stdlib.h>
#include <Wire.h>

#include "ANX.h"
#include "anim.h"
#include "buttons.h"
#include "chat.h"
#include "flash.h"
#include "games.h"
#include "graphics.h"
#include "input.h"
#include "life.h"
#include "menu.h"
#include "ninja.h"
#include "pong.h"
#include "rf.h"
#include "serial.h"
#include "settings.h"
#include "strings.h"

#define OLED_DC       PB3
#define OLED_RESET    PB4
#define OLED_CS       PB12
Adafruit_SSD1306      display(OLED_DC, OLED_RESET, OLED_CS);

#define SPLASH_DELAY  1000

//SPI Flash
SPIClass              spi2(2);
ANXFlash              flash = ANXFlash(PB9, &spi2);

//LEDs
#define LED_PIN       PA0
Adafruit_NeoPixel     leds = Adafruit_NeoPixel(NUMBER_LEDS, PA0, NEO_GRB + NEO_KHZ800);

//Radio
RFM69 radio =         RFM69(RFM69_CS, RFM69_INT, true, RFM69_INT_NUM);

//Create the menu
ANDnXORMenu menu = ANDnXORMenu();

/**
   Setup the badge, init the peripherals before going into main loop
*/
void setup()   {
  //Setup SPI2 bus, this needs to match what's in teh SSD1306 library
  spi2.setDataMode(0);
  spi2.setBitOrder(MSBFIRST);
  spi2.setClockDivider(SPI_CLOCK_DIV32);

  //Compiler enables full debug pins but we only need SWD. In fact, JTAG debug pins conflict
  //with the OLED RESET and OLED DC pins.
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);

  //Disable peripheral(s) for lower power
  rcc_clk_disable(RCC_ADC2);
  rcc_clk_disable(RCC_ADC3);
  rcc_clk_disable(RCC_BKP);
  rcc_clk_disable(RCC_CRC);
  rcc_clk_disable(RCC_DAC);
  rcc_clk_disable(RCC_DMA1);
  rcc_clk_disable(RCC_DMA2);
  rcc_clk_disable(RCC_FLITF);
  rcc_clk_disable(RCC_FSMC);
  rcc_clk_disable(RCC_I2C1);
  rcc_clk_disable(RCC_PWR);
  rcc_clk_disable(RCC_SDIO);
  rcc_clk_disable(RCC_SPI1);
  rcc_clk_disable(RCC_SPI3);
  rcc_clk_disable(RCC_TIMER3);
  rcc_clk_disable(RCC_TIMER4);
  rcc_clk_disable(RCC_TIMER5);
  rcc_clk_disable(RCC_TIMER6);
  rcc_clk_disable(RCC_TIMER7);
  rcc_clk_disable(RCC_TIMER8);
  rcc_clk_disable(RCC_TIMER9);
  rcc_clk_disable(RCC_TIMER10);
  rcc_clk_disable(RCC_TIMER11);
  rcc_clk_disable(RCC_TIMER12);
  rcc_clk_disable(RCC_TIMER13);
  rcc_clk_disable(RCC_TIMER14);
  rcc_clk_disable(RCC_UART4);
  rcc_clk_disable(RCC_UART5);
  rcc_clk_disable(RCC_I2C2);
  rcc_clk_disable(RCC_USART2);

  //Enable Serial
  pinMode(PA2, OUTPUT);
  digitalWrite(PA2, HIGH);
  Serial.begin(115200);

  //Seed random with some entropy
  randomSeed(rtMillis());  //Pretty lame seed eh?

  //Setup SPI Flash
  flash.begin();

  //Setup RF
  ANXRFBegin();
  delay(500);

  //Setup buttons
  setupButtons();

  //Setup Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //Rotate display 180 degrees
  display.ssd1306_command(SSD1306_SEGREMAP);
  display.ssd1306_command(SSD1306_COMSCANINC);
  //Reduce Power
  display.dim(true);
  //Setup display text
  display.setTextColor(INVERSE);
  display.setTextSize(1);
  display.setTextWrap(false);

  //
  //  //If RF is not ready, ask the user to hard reset
  //  if (!ANXRFAvailable()) {
  //    statusDialog("Radio Failure\nRemove & Replace\nbattery to\nreset");
  //    safeWaitForButton();
  //  }

  //Initialize the settings. This will load defaults and cache settings
  //to avoid too many reads/writes to NAND flash
  initSettings();

  //Setup LEDs
  leds.begin();
  //Keep initial brightness low then let light sense adjust from there
  leds.setBrightness(70);

  //Read Light level
  handleBrightness();

  //Check flash version, if incorrect drop into serial mode
  //Serial mode is a basic test of LEDs and display and
  //should ease flashing of 100+ badges
  if (getFlashVersion() < MIN_FLASH_VERSION || getFlashVersion() > 20) {
    doSerial();
  }

  splash();
  ANXRFSendHello();
}

/**
   Main loop of the program
*/
void loop() {
  mainMenu();
}

/**
   Allow user to edit their name
*/
void editUsername() {
  char username[ANX_INPUT_MAX];
  memset(username, '\0', ANX_INPUT_MAX);
  ANXInputWindow(username, "Name", USERNAME_MAX_LENGTH);

  //Ensure username is long enough
  if (strlen(username) < USERNAME_MIN_LENGTH) {
    statusDialog("Name too short");
    waitForButton();
    clearButtonState();
    return;
  }

  char buffer[USERNAME_MAX_LENGTH + 20];
  sprintf(buffer, "Change Name to: %s?", username);
  if (yesNoDialog(buffer)) {
    ANXSetUsername(username);
    statusDialog("Name Set");
    waitForButton();
    clearButtonState();
  }
}

/**
   Display splash screen
*/
void splash() {
  //Clear the screen, sometimes data gets left behind in the display
  display.clearDisplay();
  safeDisplay();

  bitmap bmp = getBitmapMetadata(ANDnXOR_address);

  //Draw static logo
  uint8_t x = (display.width() - bmp.width) / 2;
  uint8_t y = (display.height() - bmp.height) / 2;
  drawBitmapFlash(bmp, x, y);
  safeDisplay();

  leds.setBrightness(40);

  //Test the LEDs
  for (uint8_t c = 0; c < 7; c++) {
    setAllLeds(roygbiv[c]);
    deepSleep(200);
  }
  ledsOff();

  //Draw bender
  display.clearDisplay();
  bitmap bender = getBitmapMetadata(BENDER_address);
  x = (display.width() - bender.width) / 2;
  y = (display.height() - bender.height) / 2;
  drawBitmapFlash(bender, x, y);
  safeDisplay();

  //Glow the eyes
  float hue = 0;
  while (getButtonState() == 0) {
    uint32_t rgb = HSVtoRGB(hue, 1, 1);
    setEyeLeftLed(rgb);
    setEyeRightLed(rgb);

    hue += .001;
    if (hue >= 1) hue -= 1;

    deepSleep(50);
  }

  clearButtonState();
}
