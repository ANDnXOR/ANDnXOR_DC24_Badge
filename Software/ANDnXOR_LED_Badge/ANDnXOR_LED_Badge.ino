#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Arduino.h>
#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <libmaple/usb.h>
#include <RTClock.h>

//LEDs
#define LED_PIN       PA0
#define NUMBER_LEDS   8
Adafruit_NeoPixel     leds = Adafruit_NeoPixel(NUMBER_LEDS, PA0, NEO_GRB + NEO_KHZ800);

//LSENSE
#define LSENSE_ANODE          PB1
#define LSENSE_CATHODE        PB0
#define LSENSE_PERIOD         2000
#define MIN_BRIGHTNESS        7
#define MAX_BRIGHTNESS        70
uint32_t lastLightSense =     0;


//Real Time Clock
#define RT_MS_SCALE    19
RTClock rt(RTCSEL_LSI, 1);
static bool alarmCreated = false;

#define TIME_PER_MODE       30000

//ROYGBIV
uint32_t roygbiv[7] = {
  leds.Color(255, 0, 0), leds.Color(255, 80, 0), leds.Color(255, 255, 0),
  leds.Color(0, 255, 0), leds.Color(65, 65, 190), leds.Color(0, 0, 100),
  leds.Color(80, 0, 120)
};

void setup() {
  rcc_switch_sysclk(RCC_CLKSRC_HSE);
  rcc_set_prescaler(RCC_PRESCALER_USB, RCC_USB_SYSCLK_DIV_1_5);
  rcc_clk_init(RCC_CLKSRC_HSE, RCC_PLLSRC_HSE , RCC_PLLMUL_9);
  rcc_switch_sysclk(RCC_CLKSRC_PLL);

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
  rcc_clk_disable(RCC_SPI2);
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

  //Setup LEDs
  leds.begin();
  //Keep initial brightness low then let light sense adjust from there
  leds.setBrightness(30);

  //Make sure light levels are right
  handleBrightness();

  for (int i = 0; i < 7; i++) {
    setAllLeds(roygbiv[i]);
    deepSleep(200);
  }
}

void loop() {
  //Force background tasks before we show anything to the user to avoid
  //LEDs from being too bright at the beginning
  tick();

  columns();
  flames();
  rainbowSnake();
  snake();
  eyes();
  rainbow();
  nayan();
}

/**
   Perform a deep sleep for set period of time in ms. This isn't garanteed to be accurate
   as it may be interrupted. It will make best effort to get close.
*/
void deepSleep(uint32_t ms) {
  uint32_t startTime = rtMillis();
  while ((rtMillis() - startTime) < ms) {
    //limit sleep time to 300ms at a time to allow RF handling
    uint32_t sleepTime = min(ms, 300);
    //clean up any remaining sleep time
    sleepTime = min(sleepTime, ms - rtMillis() - startTime);
    deepSleepInterruptable(sleepTime);
  }
}

/**
   Put the MCU into a very deep sleep (stop) until there's and external interrupt or RTC alarm
*/
void deepSleepInterruptable(uint32_t ms) {


  if (!alarmCreated) {
    rt.createAlarm(&wakeup, rt.getTime() + (RT_MS_SCALE * ms));
    alarmCreated = true;
  } else {
    rt.setAlarmTime(rt.getTime() + (RT_MS_SCALE * ms));
  }

  bool usb = usb_is_connected(USBLIB);

  if (!usb) {
    //Clear the LPDS and PDDS bits
    //    PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;

    //To Enter STOP mode:
    //WFI (Wait for Interrupt) or WFE (Wait for Event) while:
    //– Set SLEEPDEEP bit in Cortex-M3 System Control register
    //    SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

    //– Clear PDDS bit in Power Control register (PWR_CR)
    PWR_BASE->CR &= ~PWR_CR_PDDS;

    //– Select the voltage regulator mode by configuring LPDS bit in PWR_CR
    //         0=run regulator even in stop mode, 1=low power regulator
    //    PWR_BASE->CR &= ~PWR_CR_LPDS;

    // Enable the wakeup flag
    PWR_BASE->CR |= PWR_CR_CWUF;

    // Enable wakeup pin bit.
    PWR_BASE->CR |=  PWR_CSR_EWUP;
  }
  //Wait for Interrupt
  asm("    wfi");

  if (!usb) {
    //Go back to normal run mode
    //    SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;
    //Turn on HSE clock
    rcc_turn_on_clk(RCC_CLK_HSE);

    // Attempt to set up 72MHz and 1x USB clock
    rcc_switch_sysclk(RCC_CLKSRC_HSE);
    rcc_set_prescaler(RCC_PRESCALER_USB, RCC_USB_SYSCLK_DIV_1_5);
    rcc_clk_init(RCC_CLKSRC_HSE, RCC_PLLSRC_HSE , RCC_PLLMUL_9);
    rcc_switch_sysclk(RCC_CLKSRC_PLL);
  }
}


/**
   Sense light level returning a value between 0 and 100
*/
uint8_t getLightLevel() {
  noInterrupts();

  pinMode(LSENSE_ANODE, OUTPUT);
  pinMode(LSENSE_CATHODE, OUTPUT);

  //Reverse bias the LED
  digitalWrite(LSENSE_ANODE, LOW);
  digitalWrite(LSENSE_CATHODE, HIGH);

  //Isolate cathode
  pinMode(LSENSE_CATHODE, INPUT);
  //Back to normal
  digitalWrite(LSENSE_CATHODE, LOW);

  //Read the time it takes to discharge the LED, longer time = darker environment
  uint32_t startTime = rtMillis();
  uint32_t delta = 0;

  //Wait for cathode to go to 0
  while (digitalRead(LSENSE_CATHODE) > 0 && delta < 20) {
    delta = rtMillis() - startTime;
  }
  interrupts();

  return map(delta, 20, 0, 0, 100);
}

/**
   Handle led brightness based on current light level and user settings
*/
void handleBrightness() {

  if (rtMillis() - lastLightSense > LSENSE_PERIOD) {
    uint8_t level = getLightLevel();
    uint8_t brightness = map(level, 0, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    leds.setBrightness(brightness);
    leds.show();
    lastLightSense = rtMillis();
  }
}


/**
   Convert HSV value to RGB 32-bit value
   H, S, V must be 0 to 1
*/
uint32_t HSVtoRGB(float H, float S, float V) {
  float h = H * 6;
  uint8_t i = floor(h);
  float a = V * (1 - S);
  float b = V * (1 - S * (h - i));
  float c = V * (1 - (S * (1 - (h - i))));
  float rf, gf, bf;

  switch (i) {
    case 0:
      rf = V * 255;
      gf = c * 255;
      bf = a * 255;
      break;
    case 1:
      rf = b * 255;
      gf = V * 255;
      bf = a * 255;
      break;
    case 2:
      rf = a * 255;
      gf = V * 255;
      bf = c * 255;
      break;
    case 3:
      rf = a * 255;
      gf = b * 255;
      bf = V * 255;
      break;
    case 4:
      rf = c * 255;
      gf = a * 255;
      bf = V * 255;
      break;
    case 5:
      rf = V * 255;
      gf = a * 255;
      bf = b * 255;
      break;
  }

  uint8_t R = rf;
  uint8_t G = gf;
  uint8_t B = bf;

  uint32_t RGB = (R << 16) + (G << 8) + B;
  return RGB;
}

/**
   Find a random brightness up to color c
*/
uint32_t randBrightness(uint32_t c) {
  uint8_t r = c >> 16;
  uint8_t g = (c >> 8) & 0xFF;
  uint8_t b = c & 0xFF;
  uint8_t f = random(1, 5);

  c = ((r / f) << 16) | ((g / f) << 8) | (b / f);
  return c;
}

/**
   Get approximate ms count from the RTC
*/
uint32_t rtMillis() {
  return rt.getTime() / RT_MS_SCALE;
}

/**
   Set all LEDS to a color
*/
void setAllLeds(uint32_t color) {
  for (uint8_t i = 0; i < NUMBER_LEDS; i++) {
    leds.setPixelColor(i, color);
  }
  leds.show();
}

/**
   Set all LEDS to a color
*/
void setAllLeds(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < NUMBER_LEDS; i++) {
    leds.setPixelColor(i, r, g, b);
  }
  leds.show();
}

void columns() {
  uint32_t endtime = rtMillis() + TIME_PER_MODE;
  uint8_t leftCol[] = {7, 0, 1, 2};
  uint8_t rightCol[] = {3, 4, 5, 6};
  uint8_t index = 0;
  uint8_t colorIndex = 0;
  uint32_t color = roygbiv[colorIndex]; //green

  while (rtMillis() < endtime) {
    leds.setPixelColor(leftCol[index], 255, 255, 255);
    leds.setPixelColor(rightCol[index], 255, 255, 255);
    leds.show();
    deepSleep(40);

    leds.setPixelColor(leftCol[index], color);
    leds.setPixelColor(rightCol[index], color);
    leds.show();

    index++;
    if (index == 4) {
      index = 0;
      colorIndex = (colorIndex + 1) % 7;
      color = roygbiv[colorIndex];
    }

    deepSleep(100);
    tick();
  }
}

void eyes() {
  float hue = 0;
  uint32_t endtime = rtMillis() + TIME_PER_MODE;
  setAllLeds(0);

  while (rtMillis() < endtime) {
    uint32_t color = HSVtoRGB(hue, 1, 1);

    leds.setPixelColor(0, color);
    leds.setPixelColor(5, color);
    leds.show();

    deepSleep(50);
    tick();

    hue += .01;
    if (hue >= 1) hue -= 1;
  }
}

void flames() {
  uint32_t endtime = rtMillis() + TIME_PER_MODE;

  while (rtMillis() < endtime) {
    //red
    leds.setPixelColor(2, randBrightness(roygbiv[0]));
    leds.setPixelColor(3, randBrightness(roygbiv[0]));
    leds.setPixelColor(1, randBrightness(roygbiv[0]));
    leds.setPixelColor(4, randBrightness(roygbiv[0]));
    //orange
    leds.setPixelColor(0, randBrightness(roygbiv[1]));
    leds.setPixelColor(5, randBrightness(roygbiv[1]));
    //yellow
    leds.setPixelColor(6, randBrightness(roygbiv[2]));
    leds.setPixelColor(7, randBrightness(roygbiv[2]));
    leds.show();

    deepSleep(200);
    tick();
  }
}

void nayan() {
  uint32_t endtime = rtMillis() + TIME_PER_MODE;
  uint8_t c = 0;

  while (rtMillis() < endtime) {
    uint8_t i = random(NUMBER_LEDS);
    leds.setPixelColor(i, 255, 255, 255);
    leds.show();
    deepSleep(60);

    leds.setPixelColor(i, roygbiv[c]);
    leds.show();

    c = (c + 1) % 7;

    deepSleep(100);
  }
}

void rainbow() {
  float hue = 0;

  //Define rows of leds
  uint8_t rows[4][2] = {
    {7, 6},
    {0, 5},
    {1, 4},
    {2, 3}
  };

  uint32_t endtime = rtMillis() + TIME_PER_MODE;

  while (rtMillis() < endtime) {
    for (uint8_t row = 0; row < 4; row++) {
      float rowhue = hue + (row * .25);
      if (rowhue >= 1) rowhue -= 1;
      uint32_t color = HSVtoRGB(rowhue, 1, 1);

      leds.setPixelColor(rows[row][0], color);
      leds.setPixelColor(rows[row][1], color);
    }
    leds.show();

    //Increment row and color and loop around
    hue += .01;
    if (hue >= 1) hue = 0;

    deepSleep(70);
    tick();
  }
}

void rainbowSnake() {
  uint8_t ledOrder[] = {7, 0, 1, 2, 3, 4, 5, 6};
  float hue = 0;
  uint8_t i = 0;
  uint32_t endtime = rtMillis() + TIME_PER_MODE;

  while (rtMillis() < endtime) {
    leds.setPixelColor(i, 255, 255, 255);
    leds.show();
    deepSleep(60);

    leds.setPixelColor(ledOrder[i], HSVtoRGB(hue, 1, 1));
    leds.show();

    i = (i + 1) % NUMBER_LEDS;
    hue += .1;
    if (hue >= 1) hue -= 1;

    deepSleep(40);
    tick();
  }
}

void snake() {
  uint8_t ledOrder[] = {7, 0, 1, 2, 3, 4, 5, 6};
  uint32_t lastLed = 0;
  uint8_t ledDelay = 50;
  uint8_t c = 0;
  uint32_t endtime = rtMillis() + TIME_PER_MODE;

  while (rtMillis() < endtime) {
    for (uint8_t i = 0; i < NUMBER_LEDS; i++) {
      lastLed = rtMillis();
      leds.setPixelColor(ledOrder[i], roygbiv[c]);
      leds.show();
      deepSleep(50);
    }

    //Handle tasks
    tick();

    //Next color
    c = (c + 1) % 7;
  }
}

/**
   Called periodically to handle tasks like RF and light sensor
*/
void tick() {
  handleBrightness();
}

/**
   Handles sleep wakeups by doing nothing. Anything that occurs here will happen without a 72mhz clock!
*/
void wakeup() {
  //NOP - unsafe to do most things since clock is not fully available
}
