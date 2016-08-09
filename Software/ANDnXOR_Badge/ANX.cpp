#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <Arduino.h>
#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <libmaple/usb.h>
#include <RFM69registers-ANDnXOR.h>
#include <RTClock.h>
#include "ANX.h"
#include "buttons.h"
#include "input.h"
#include "flash.h"
#include "rf.h"
#include "serial.h"

extern Adafruit_SSD1306   display;
extern Adafruit_NeoPixel  leds;
extern ANXFlash           flash;

bool displayInverted = false;

//Chat stuff
ChatMessage chatBuffer[CHAT_MESSAGES_MAX];
int chatBufferPtr;

char USERNAME_DEFAULT[] = "N00B";
bool chatToTerminal = false; //Should all chat activity be dumped to terminal?

//Real Time Clock
RTClock rt(RTCSEL_LSI, 1);
static bool alarmCreated = false;

//Input settings
char INPUT_CHARS[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.!?,()[]{}<>/\\|;:";

//Light Sense
uint32_t lastLightSense = rtMillis();

//Screen flip
uint32_t lastTiltSense = rtMillis();

static int16_t nodeId = -1;
uint8_t XP_PER_LEVEL = 100;

//ROYGBIV
uint32_t roygbiv[7] = {
  leds.Color(255, 0, 0), leds.Color(255, 80, 0), leds.Color(255, 255, 0),
  leds.Color(0, 255, 0), leds.Color(65, 65, 190), leds.Color(0, 0, 100),
  leds.Color(80, 0, 120)
};


/**
   Display about window
*/
void about() {
  int16_t offset = 0;
  //Really large scroll area
  int16_t minOffset = 0 - 1000 + display.height();

  //Loop forever so we can handle button presses by scrolling
  while (1) {
    display.clearDisplay();

    //Software versions
    display.setCursor(1, offset);
    display.print("Software:");
    printAlignRight(VERSION, ANX_FONT_HEIGHT + offset);
    display.setCursor(1, (2 * ANX_FONT_HEIGHT) + offset);
    display.print("Flash Data:");
    char buff[8];
    sprintf(buff, "v%d\0", getFlashVersion());
    printAlignRight(buff, (3 * ANX_FONT_HEIGHT) + offset);

    //Credits
    display.setCursor(1, (5 * ANX_FONT_HEIGHT) + offset);
    display.print("Brought to you by:");
    printAlignRight("@ANDnXOR", (6 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("@lacosteaef", (7 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("@andrewnriley", (8 * ANX_FONT_HEIGHT) + offset);

    //Special thanks
    display.setCursor(1, (10 * ANX_FONT_HEIGHT) + offset);
    display.print("Special Thanks To:");
    printAlignRight("true",  (11 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("anarduino.com", (12 * ANX_FONT_HEIGHT) + offset);

    //Github
    display.setCursor(1, (14 * ANX_FONT_HEIGHT) + offset);
    display.print("Github:");
    printAlignRight("http://bit.ly/27qkhBm", (15 * ANX_FONT_HEIGHT) + offset);

    //Open source credits
    display.setCursor(1, (17 * ANX_FONT_HEIGHT) + offset);
    display.print("Open Source:");
    printAlignRight("STM32Duino", (18 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("LeafLabs Maple", (19 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("Adafruit SSD1306", (20 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("Adafruit Neopixel", (21 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("Adafruit GFX", (22 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("LowPowerLab RFM69", (23 * ANX_FONT_HEIGHT) + offset);
    printAlignRight("Kicad", (24 * ANX_FONT_HEIGHT) + offset);

    //Scroll unlock stuffs
    display.setCursor(1, 300 + offset);
    display.print("DEFCON 24");
    display.setCursor(1, 600 + offset);
    display.print("There's nothing down\nhere");
    display.setCursor(1, 800 + offset);
    display.print("Seriously, stop it!");
    display.setCursor(1, 1000 - ANX_FONT_HEIGHT + offset);
    display.print("Badge unlocked");

    safeDisplay();

    uint8_t button = waitForButton();
    if (button > 0) {
      if (button == BUTTON_LEFT) {
        clearButtonState();
        return;
      } else if (button == BUTTON_DOWN) {
        offset -= ANX_FONT_HEIGHT / 2;
      } else if (button == BUTTON_UP) {
        offset += ANX_FONT_HEIGHT / 2;
      }

      if (offset <= minOffset) {
        offset = minOffset;

        //unlock the badge if they scroll way down
        if ((ANXGetUnlocked() & UNLOCK_SCROLL) == 0) {
          addExperience(XP_PER_LEVEL);
          ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_SCROLL);
        }
      }

      if (offset > 0) offset = 0;
      deepSleep(BUTTON_REPEAT_DELAY);
    }
  }
}

/**
   Give the user some experience
*/
void addExperience(uint32_t xp) {
  uint32_t newXP = ANXGetExperience() + xp;
  while (newXP >= XP_PER_LEVEL) {
    ANXSetLevel(ANXGetLevel() + 1);
    newXP -= XP_PER_LEVEL;
  }
  ANXSetExperience(newXP);
}

/**
   Push a message onto the chat buffer
*/
void addToChatBuffer(ChatMessage message) {
  chatBuffer[chatBufferPtr] = message;
  chatBufferPtr = (chatBufferPtr + 1) % CHAT_MESSAGES_MAX;
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
   Draw bitmap from memory
*/
void drawBitmap(bitmap bmp, uint8_t *pixels, int16_t x, int16_t y, uint8_t color) {
  int16_t i, j;
  uint16_t offset = 0;

  uint8_t b = pixels[offset];
  offset++;
  int8_t m = 7;

  //Walk through pixels one at a time drawing
  for (j = 0; j < bmp.height; j++) {
    for (i = 0; i < bmp.width; i++) {
      if ((b >> m) & 0x01) {
        display.drawPixel(x + i, y + j, color);
      }

      m--;
      if (m < 0) {
        m = 7;
        b = pixels[offset];
        offset++;
      }
    }
  }
}

/**
   Draw a bitmap from flash memory to the screen
   Bitmap struct must point to valid bitmap on flash storage
   Bitmap is simple, 1 = WHITE, 0 = BLACK
*/
void drawBitmapFlash(bitmap bmp, int16_t x, int16_t y) {
  drawBitmapFlash(bmp, x, y, WHITE);
}

/**
   Draw a bitmap from flash memory to the screen
   Bitmap struct must point to valid bitmap on flash storage
   Bitmap is simple, 1 = color, 0 = BLACK
*/
void drawBitmapFlash(bitmap bmp, int16_t x, int16_t y, uint8_t color) {
  uint16_t s = (uint16_t)ceil((bmp.width * bmp.height) / 8.0);
  uint8_t pixels[s];

  //Read pixel data into memory
  flash.readBytes(bmp.address, pixels, s);

  drawBitmap(bmp, pixels, x, y, color);
}

/**
   Read metadata for a bitmap from flash
*/
bitmap getBitmapMetadata(uint32_t address) {
  byte bytes[8];

  //Read raw metadata from flash
  flash.readBytes(address, bytes, 8);

  return {address + 256, bytes[0], bytes[1], bytes[2]};
}

/**
   Reads the version number of the flash data
*/
uint8_t getFlashVersion() {
  return flash.readByte(FLASH_VERSION_ADDRESS);
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

  //  if (delta == 0) {
  //    return 0;
  //  }

  return map(delta, 20, 0, 0, 100);
}

/**
   Get node ID from flash
*/
byte getNodeID() {
  if (nodeId == -1) {
    byte id = flash.readByte(NODE_ID_ADDRESS);
    if (id == 0xFF) id = 1;
    nodeId = id;
  }
  return nodeId;
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
   Get index of char c in text
*/
int16_t ANXIndexOf(char *text, char c) {
  for (uint8_t i = 0; i < strlen(text); i++) {
    if (text[i] == c) {
      return i;
    }
  }
  return -1;
}

/**
   Turn off all LEDs.
*/
void ledsOff() {
  setAllLeds(0, 0, 0);
}

/**
   Play an animation forever, or at least until something is pressed
*/
uint8_t playAnimation(uint32_t address, uint8_t d, void (*frameCallback)(uint8_t frame, uint8_t *data)) {
  playAnimation(address, d, frameCallback, true);
}

/**
   Play an animation
   Returns the button state that caused it to stop
*/
uint8_t playAnimation(uint32_t address, uint8_t d, void (*frameCallback)(uint8_t frame, uint8_t *data), bool loop) {
  bitmap bmp;

  //Handle if empty address was sent it
  if (address == 0x0) {
    bmp.address = 0x0;
    bmp.frames = 1;
    display.clearDisplay();
    safeDisplay();
  } else {
    bmp = getBitmapMetadata(address);
  }

  //Current frame index
  uint8_t i = 0;
  uint8_t data = 0;

  //Center animation
  uint8_t x = (display.width() - bmp.width) / 2;
  uint8_t y = (display.height() - bmp.height) / 2;

  //Calculate size and page count
  uint16_t frameSize = (uint16_t)ceil((bmp.width * bmp.height) / 8.0);
  uint8_t pagesPerFrame = ceil(frameSize / 256.0);


  while (1) {

    //Draw the current frame, if it has a valid address
    if (bmp.address > 0x0) {
      display.clearDisplay();
      bitmap frame = {bmp.address + (i * pagesPerFrame * 256), bmp.width, bmp.height, bmp.frames};
      drawBitmapFlash(frame, x, y);
      safeDisplay();
    }

    //Advance the frame index wrapping around if necessary
    i++;
    if (i >= bmp.frames) {
      if (loop) {
        i = 0;
      } else {
        return 0;
      }
    }

    //Make sure we're handling tasks every so often and not sleeping too much
    for (uint16_t i = 0; i < d; i += 50) {
      deepSleep(50);
      tick();
    }

    //Callback every frame
    if (frameCallback != NULL) {
      frameCallback(i, &data);
    }

    //Handle buttons
    uint8_t button = getButtonState();
    if (getButtonState() > 0) {
      safeClearButtonState();
      ledsOff();
      return button;
    }
  }
}

/**
   Print text at y aligned to the right side of the screen
*/
void printAlignRight(char *message, int16_t y) {
  uint8_t w = strlen(message) * ANX_FONT_WIDTH;
  uint8_t x = display.width() - w - 1;
  display.setCursor(x, y);
  display.print(message);
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
   Use the RTC to delay safely
*/
void rtDelay(uint32_t ms) {
  uint32_t start = rtMillis();
  //Hang out until it's time to quit
  while ((rtMillis() - start) < ms);
}

/**
   Get approximate ms count from the RTC
*/
uint32_t rtMillis() {
  return rt.getTime() / RT_MS_SCALE;
}

/**
   Push bits to the display safely
*/
void safeDisplay() {
  noInterrupts();
  display.display();
  interrupts();
}

/**
   Self test UI
*/
void selfTest() {
  bool radioAvailable = false;
  bool radioMode = false;
  bool radioReady = false;
  bool flashAvailable = false;
  bool flashRead = false;

  uint8_t y;
  char *pass = "PASS";
  char *fail = "FAIL";

  setAllLeds(255, 255, 255);

  while (1) {
    y = ANX_FONT_HEIGHT + 3;
    radioAvailable = ANXRFAvailable();
    radioMode = (radio.readReg(REG_OPMODE) & RF_OPMODE_SYNTHESIZER) == 0;
    flashAvailable = (flash.getManufacturerId()) == 1; //Spansion manufacturer ID
    flashRead = (getFlashVersion() > 0) && (getFlashVersion() < 20);

    window("Self Test");

    //Radio available, basic availability check from RF
    display.setCursor(1, y);
    display.print("Radio Avail");
    printAlignRight(radioAvailable ? pass : fail, y);

    //Radio mode status, frequency synth = fail, indicates possible stuck PLL
    y += ANX_FONT_HEIGHT;
    display.setCursor(1, y);
    display.print("Radio Mode");
    printAlignRight(radioMode ? pass : fail, y);

    //Light sensor reading
    y += ANX_FONT_HEIGHT;
    display.setCursor(1, y);
    display.print("Light Level");
    char light[3];
    sprintf(light, "%d", getLightLevel());
    printAlignRight(light, y);

    //Flash manufacturer ID is readable
    y += ANX_FONT_HEIGHT;
    display.setCursor(1, y);
    display.print("Flash Available");
    printAlignRight(flashAvailable ? pass : fail, y);

    //Flash data version falls in a reasonable range (able to read flash accuratly)
    y += ANX_FONT_HEIGHT;
    display.setCursor(1, y);
    display.print("Flash Read");
    printAlignRight(flashRead ? pass : fail, y);

    //Just print the node id and last byte of the serial
    y += ANX_FONT_HEIGHT;
    display.setCursor(1, y);
    unsigned long *unique = (unsigned long *)0x1FFFF7E8;
    display.print("0x");
    display.print(unique[2], HEX);
    char id[3];
    sprintf(id, "%d", getNodeID());
    printAlignRight(id, y);

    safeDisplay();

    deepSleep(100);
    //Handle tasks
    tick();

    uint8_t button = getButtonState();
    if (button > 0) {
      clearButtonState();
      ledsOff();
      return;
    }
  }
}

/**
   UI for sending an alert
*/
#ifdef MASTER
void sendAlert() {
  if (!ANXRFAvailable()) {
    statusDialog("Radio disabled");
    waitForButton();
    clearButtonState();
    return;
  }

  char message[ANX_ALERT_MAX_LENGTH];
  memset(message, '\0', ANX_ALERT_MAX_LENGTH);
  ANXInputWindow(message, "Send Alert", ANX_ALERT_MAX_LENGTH);
  if (strlen(message) > 0) {
    char buffer[ANX_ALERT_MAX_LENGTH + 8];
    sprintf(buffer, "Send?\n%s", message);
    if (yesNoDialog(buffer)) {
      ANXRFSendAlert(message);
      statusDialog("Alert Sent");
      waitForButton();
      clearButtonState();
      return;
    }
  }

  statusDialog("Cancelled");
  waitForButton();
  clearButtonState();
}
#endif

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

/**
   Set left eye color
*/
void setEyeLeftLed(uint32_t color) {
  if (ANXGetEyes()) {
    leds.setPixelColor(0, color);
  } else {
    leds.setPixelColor(0, 0, 0, 0);
  }
  leds.show();
}


/**
   Set left eye color
*/
void setEyeLeftLed(uint8_t r, uint8_t g, uint8_t b) {
  if (ANXGetEyes()) {
    leds.setPixelColor(0, r, g, b);
  } else {
    leds.setPixelColor(0, 0, 0, 0);
  }
  leds.show();
}

/**
   Set right eye color
*/
void setEyeRightLed(uint8_t r, uint8_t g, uint8_t b) {
  if (ANXGetEyes()) {
    leds.setPixelColor(5, r, g, b);
  } else {
    leds.setPixelColor(5, 0, 0, 0);
  }
  leds.show();
}

/**
   Set right eye color
*/
extern void setEyeRightLed(uint32_t color) {
  if (ANXGetEyes()) {
    leds.setPixelColor(5, color);
  } else {
    leds.setPixelColor(5, 0, 0, 0);
  }
  leds.show();
}

/**
   Draw a slider at X,Y and handle user setting values
   Return the selected value upon enter.
*/
int slider(uint8_t x, uint8_t y, int min, int max, int value) {
  uint8_t w = display.width() - (2 * x);
  uint8_t h = 10;
  uint8_t step = (max - min) / 10;

  while (1) {
    uint8_t valuew = (uint8_t)(((float)(value - min) / (float)(max - min)) * w);

    //clear the slider area
    display.fillRoundRect(x, y, w, h, 2, BLACK);

    //Draw the slider
    display.drawRoundRect(x, y, w, h, 2, WHITE);
    display.fillRoundRect(x, y, valuew, h, 2, WHITE);

    safeDisplay();

    uint8_t button = waitForButton();
    clearButtonState();
    if (button == BUTTON_LEFT) {
      value -= step; if (value < min) value = min;
    } else if (button == BUTTON_RIGHT) {
      value += step; if (value > max) value = max;
    } else if (button == BUTTON_ENTER) {
      return value;
    }
  }
}

/**
   Generate a status dialog on the screen
*/
void statusDialog(char *message) {
  uint8_t w = display.width() * .8;
  uint8_t h = display.height() * .8;
  uint8_t x = (display.width() - w) / 2;
  uint8_t y = (display.height() - h) / 2;
  uint8_t charsPerRow = (w - 3) / (ANX_FONT_WIDTH);
  uint8_t lines = 1;

  //Count newlines
  for (uint8_t i = 0; i < strlen(message); i++) {
    if (message[i] == '\n')
      lines++;
  }

  //Center the message
  uint8_t tw = strlen(message) * ANX_FONT_WIDTH;
  uint8_t tx = (display.width() - tw) / 2;
  uint8_t ty = (display.height() - (ANX_FONT_HEIGHT * lines)) / 2;

  //Draw the window
  display.fillRoundRect(x - 1, y - 1, w + 2, h + 2, 2, BLACK);
  display.drawRoundRect(x, y, w, h, 2, WHITE);

  //Print the message one line at a time
  char line[32];
  memset(line, '\0', 32);
  uint8_t index = 0;

  //Walk through the message printing one line at a time
  for (uint8_t i = 0; i < strlen(message); i++) {
    //Build a line of text
    if (message[i] != '\n') {
      line[index] = message[i];
      index++;
    } else {
      //This is a new line, center the text
      uint8_t tx = (display.width() - (index * ANX_FONT_WIDTH)) / 2;
      display.setCursor(tx, ty);

      //Print the current line
      display.print(line);

      //Reset
      index = 0;
      memset(line, '\0', 32);

      //Move the cursor
      ty += ANX_FONT_HEIGHT;
    }
  }

  //Print any remaining text
  if (index > 0) {
    uint8_t tx = (display.width() - (index * ANX_FONT_WIDTH)) / 2;
    display.setCursor(tx, ty);

    //Print the current line
    display.print(line);
  }

  //Clear any previous button before showing dialog
  clearButtonState();

  safeDisplay();
}

/**
   Called periodically to handle tasks like RF and light sensor
*/
void tick() {
  handleBrightness();
  rfHandler();
  tiltHandler();
}

/**
   Detect tilt changes in badge
*/
void tiltHandler() {

  //Disable tilt
  if (!ANXGetTilt()) {
    return;
  }

  //Quit if it's too soon
  if (rtMillis() - lastTiltSense < TILT_PERIOD) {
    return;
  }

  if (displayInverted) {
    if (digitalRead(TILT_PIN) == HIGH) {
      deepSleep(TILT_DEBOUNCE_DELAY);
      if (digitalRead(TILT_PIN) == HIGH) {
        displayInverted = false;
        noInterrupts();
        display.ssd1306_command(SSD1306_SEGREMAP);
        display.ssd1306_command(SSD1306_COMSCANINC);
        display.display();
        interrupts();
      }
    }
  } else {
    if (digitalRead(TILT_PIN) == LOW) {
      deepSleep(TILT_DEBOUNCE_DELAY);
      if (digitalRead(TILT_PIN) == LOW) {
        displayInverted = true;
        noInterrupts();
        display.ssd1306_command(0xA1);
        display.ssd1306_command(SSD1306_COMSCANDEC);
        display.display();
        interrupts();
      }
    }
  }

  lastTiltSense = rtMillis();
}

/**
   Special troll mode that trolls all badges
*/
#ifdef MASTER
void troll() {
  if (yesNoDialog("Rick Roll\nAll Humans?")) {
    //Broadcast troll mode rick roll to all badges
    ANXRFSendTrollRick(0xFF);
    statusDialog("Humans\nRick Rolled");
    safeWaitForButton();
  }
}
#endif

/**
   Handles sleep wakeups by doing nothing. Anything that occurs here will happen without a 72mhz clock!
*/
void wakeup() {
  //NOP - unsafe to do most things since clock is not fully available
}

/**
   Display a window with a given title, a window is full screen
*/
void window(char *title) {
  display.clearDisplay();

  display.setCursor(1, 1);
  display.print(title);
  display.fillRect(0, 0, display.width(), ANX_FONT_HEIGHT + 2, INVERSE);
}

/**
   Create a yes or no dialog popup
*/
bool yesNoDialog(char *message) {
  uint8_t w = display.width() * .8;
  uint8_t h = display.height() * .7;
  uint8_t x = (display.width() - w) / 2;
  uint8_t y = (display.height() - h) / 2;
  uint8_t charsPerRow = (w - 3) / (ANX_FONT_WIDTH);

  //Put the options at the bottom of the window
  char yes[] = {"Yes -->"};
  char no[] = {"<-- No"};
  uint8_t yesx = x + w - 2 - (strlen(yes) * ANX_FONT_WIDTH);
  uint8_t yesy = y + h - ANX_FONT_HEIGHT - 1;
  uint8_t nox = x + 2;
  uint8_t noy = yesy;

  //Draw the window
  display.fillRoundRect(x - 1, y - 1, w + 2, h + 2, 2, BLACK);
  display.drawRoundRect(x, y, w, h, 2, WHITE);


  //Move the cursor to the top left
  x += 2;
  y += 2 ;
  display.setCursor(x, y);
  uint8_t rowCharIndex = 0;

  //Print the message 1 char at a time
  for (uint8_t i = 0; i < strlen(message); i++) {
    display.print(message[i]);
    rowCharIndex++;

    //Wrap
    if (rowCharIndex >= charsPerRow || message[i] == '\n') {
      rowCharIndex = 0;
      y += ANX_FONT_HEIGHT;
      display.setCursor(x, y);
    }

    if (y + ANX_FONT_HEIGHT > yesy) {
      break;
    }
  }

  //Draw options
  display.setCursor(yesx, yesy);
  display.print(yes);
  display.setCursor(nox, noy);
  display.print(no);

  //Clear any previous button before showing dialog
  clearButtonState();

  safeDisplay();

  uint8_t button = 0;
  while (button != BUTTON_LEFT && button != BUTTON_RIGHT) {
    button = waitForButton();
    clearButtonState();
  }

  return (button == BUTTON_RIGHT);
}

/**
   Gets a message at index off the chat buffer
*/
ChatMessage getFromChatBuffer(uint8_t index) {
  return chatBuffer[(chatBufferPtr + index) % CHAT_MESSAGES_MAX];
}
