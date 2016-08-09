#ifndef ANX_H
#define ANX_H

#include <Arduino.h>
#include <RTClock.h>

#define ANX_DEBUG
#define MASTER 

//Version of the software
#define VERSION               "v1.3.3.7 SP3"
#define MIN_FLASH_VERSION     0xC

#define MAX_PEER_AGE          10 * 60 * 1000  //10 min
#define PEER_NODES_MAX        255

extern uint8_t XP_PER_LEVEL;

//Node ID
#define NODE_ID_ADDRESS       0x95000

//Display settings
#define ANX_FONT_WIDTH        6
#define ANX_FONT_HEIGHT       8
#define ANX_ROW_MAX_CHARS     21
extern bool displayInverted;

//Input settings
#define BUTTON_REPEAT_DELAY   140
#define ANX_INPUT_MAX         128
#define TILT_PERIOD           500
#define TILT_DEBOUNCE_DELAY   200
#define TILT_PIN              PA15
//Possible characters for entry
#define INPUT_CHARS_COUNT     55
extern char INPUT_CHARS[INPUT_CHARS_COUNT];

//LEDs
#define NUMBER_LEDS           8
#define LSENSE_ANODE          PB1
#define LSENSE_CATHODE        PB0
#define LSENSE_PERIOD         2000
#define MIN_BRIGHTNESS        7
#define MAX_BRIGHTNESS        70
extern uint32_t roygbiv[7];

//Menu values
#define MENU_BACK             -1

//Username Settings
#define USERNAME_MAX_LENGTH   8
#define USERNAME_MIN_LENGTH   3
extern char USERNAME_DEFAULT[];

//Chat Settings
#define CHAT_MESSAGES_MAX     5
#define ANX_CHAT_MAX_LENGTH   31
extern bool chatToTerminal;

//Alert Setting
#define ANX_ALERT_MAX_LENGTH  31

//RF Settings
#define PACKET_LOG_MAX        5
#define ALERT_LOG_MAX         10
#define HELLO_PERIOD          60 * 1000  //1 mins

//Internal Real Time Clock
extern RTClock rt;
#define RT_SEC_SCALE          19300
#define RT_MS_SCALE           19

//Flash Settings
#define FLASH_VERSION_ADDRESS 0x19500

//Unlock Masks
#define UNLOCK_HACKADAY       B00000001
#define UNLOCK_EFF            B00000010
#define UNLOCK_PIRATES        B00000100
#define UNLOCK_MASTER         B00001000
#define UNLOCK_ANDNXOR        B00010000
#define UNLOCK_WHOAMI         B00100000
#define UNLOCK_SCROLL         B01000000

//structure to hold bitmap metadata
struct bitmap {
  uint32_t address;
  uint8_t width;
  uint8_t height;
  uint8_t frames;
};

//Helper Functions
extern void about();
extern void addExperience(uint32_t xp);
extern void deepSleep(uint32_t ms);
extern void deepSleepInterruptable(uint32_t ms);
extern void drawBitmap(bitmap bmp, uint8_t *pixels, int16_t x, int16_t y, uint8_t color);
extern void drawBitmapFlash(bitmap bmp, int16_t x, int16_t y, uint8_t color);
extern void drawBitmapFlash(bitmap bmp, int16_t x, int16_t y);
extern void ledsOff();
extern bitmap getBitmapMetadata(uint32_t address);
extern uint8_t getFlashVersion();
extern uint8_t getLightLevel();
extern uint8_t getNodeID();
extern void handleBrightness();
extern uint32_t HSVtoRGB(float H, float S, float V);
extern int16_t ANXIndexOf(char *text, char c);
extern uint8_t playAnimation(uint32_t address, uint8_t d, void (*frameCallback)(uint8_t frame, uint8_t *data));
extern uint8_t playAnimation(uint32_t address, uint8_t d, void (*frameCallback)(uint8_t frame, uint8_t *data), bool loop);
extern void printAlignRight(char *message, int16_t y);
extern uint32_t randBrightness(uint32_t c);
extern void rtDelay(uint32_t ms);
extern uint32_t rtMillis();
extern void safeDisplay();
extern void selfTest();
#ifdef MASTER
extern void sendAlert();
#endif
extern void setAllLeds(uint32_t color);
extern void setAllLeds(uint8_t r, uint8_t g, uint8_t b);
extern void setEyeLeftLed(uint32_t color);
extern void setEyeLeftLed(uint8_t r, uint8_t g, uint8_t b);
extern void setEyeRightLed(uint32_t color);
extern void setEyeRightLed(uint8_t r, uint8_t g, uint8_t b);
extern void statusDialog(char *message);
extern void tick();
extern void tiltHandler();
#ifdef MASTER
extern void troll();
#endif
extern void wakeup();
extern void window(char *title);
extern bool yesNoDialog(char *message);

/**************************
   CHAT Stuff
 **************************/
struct ChatMessage {
  char name[USERNAME_MAX_LENGTH];
  int received = 0;
  char message[ANX_CHAT_MAX_LENGTH];
};
extern ChatMessage chatBuffer[CHAT_MESSAGES_MAX];
extern int chatBufferPtr;
extern void addToChatBuffer(ChatMessage message);
extern ChatMessage getFromChatBuffer(uint8_t index);
extern int slider(uint8_t x, uint8_t y, int min, int max, int value);

#endif
