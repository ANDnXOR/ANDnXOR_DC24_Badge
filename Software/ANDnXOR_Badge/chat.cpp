#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <Arduino.h>
#include "ANX.h"
#include "buttons.h"
#include "input.h"
#include "chat.h"
#include "rf.h"
#include "settings.h"
#include "strings.h"

extern Adafruit_SSD1306 display;

static uint8_t _bottom;
static uint8_t _maxCharsPerRow;

/**
   Entry point for chat with no quick chat
*/

void doChat() {
  //Setup chat
  _maxCharsPerRow = (display.width() / ANX_FONT_WIDTH) - 2;
  _bottom = CHAT_MESSAGES_MAX - 1;

  uint8_t button = 0;
  uint8_t lastChatPtr = 0;
  bool quit = false;

  //Warn user then quit if radio is not available
  if (!ANXRFAvailable()) {
    statusDialog("Radio disabled");
    quit = true;
  }

  //Warn user then quit if in airplane mode
  if (ANXGetAirplane()) {
    statusDialog("Badge is in\nairplane mode");
    quit = true;
  }

  if (quit) {
    waitForButton();
    clearButtonState();
    return;
  }

  while (1) {
    _drawMessages();
    lastChatPtr = chatBufferPtr;

    //Idle handling RF and buttons
    while (chatBufferPtr == lastChatPtr && button == 0) {
      //Handle tasks
      tick();
      //Check buttons
      button = getButtonState();
    }

    //Handle button presses in one block
    //If message is null, meaning nothing was passed into doChat, then execute a normal chat entry.
    if (button > 0) {
      //Quit on left
      if (button == BUTTON_LEFT) {
        clearButtonState();
        return;
      }

      //Popup chat window on enter
      if (button == BUTTON_ENTER) {
        clearButtonState();

        //Get a
        char message[ANX_CHAT_MAX_LENGTH];
        memset(message, '\0', ANX_CHAT_MAX_LENGTH); //initialize to null before using no matter wut
        ANXInputWindow(message, mi_main_chat, ANX_CHAT_MAX_LENGTH);
        if (strlen(message) > 0) {
          ANXRFSendChat(message);
        }
      }
    }
    //cleanup
    button = 0;
  }
}
/**
  Quick chat for prepending messages to doChat call.  For the ultra lazy.
**/
void doQuickChat1() {
  char message[ANX_INPUT_MAX];
  memset(message, '\0', ANX_INPUT_MAX); //initialize to null before using no matter wut
  sprintf(message, "MEET ME AT ");
  ANXInputWindow(message, mi_main_chat, ANX_CHAT_MAX_LENGTH);
  if (strlen(message) > 0) {
    ANXRFSendChat(message);
    doChat();
  }
}

void doQuickChat2() {
  char message[ANX_INPUT_MAX];
  memset(message, '\0', ANX_INPUT_MAX); //initialize to null before using no matter wut
  sprintf(message, "I'M AT ");
  ANXInputWindow(message, mi_main_chat, ANX_CHAT_MAX_LENGTH);
  if (strlen(message) > 0) {
    ANXRFSendChat(message);
    doChat();
  }
}

/**
 * Send out of beer message immediately
 */
void doQuickChat3() {
  char message[ANX_INPUT_MAX];
  memset(message, '\0', ANX_INPUT_MAX); //initialize to null before using no matter wut
  sprintf(message, "I'M OUT OF BEER, HALP!");
  ANXRFSendChat(message);
  doChat();
}

void doQuickChat4() {
  char message[ANX_INPUT_MAX];
  memset(message, '\0', ANX_INPUT_MAX); //initialize to null before using no matter wut
  sprintf(message, "FED SPOTTED AT ");
  ANXInputWindow(message, mi_main_chat, ANX_CHAT_MAX_LENGTH);
  if (strlen(message) > 0) {
    ANXRFSendChat(message);
    doChat();
  }
}

void doQuickChat5() {
  char message[ANX_INPUT_MAX];
  memset(message, '\0', ANX_INPUT_MAX); //initialize to null before using no matter wut
  sprintf(message, "A/S/L? ");
  ANXRFSendChat(message);
  doChat();
}
void _drawMessages() {
  //Move the cursor to the bottom of the screen
  display.setCursor(0, display.height());
  display.setTextWrap(true);
  int8_t row = _bottom;

  display.clearDisplay();

  //If first message is blank quit
  ChatMessage first = getFromChatBuffer(_bottom);
  if (strlen(first.name) == 0) {
    display.setCursor(1, 1);
    display.print("Nothing's here");
    safeDisplay();
    return;
  }

  //Draw each message starting from the bottom of the screen
  while (display.getCursorY() >= 0 && row >= 0) {
    ChatMessage message = getFromChatBuffer(row);

    //Stop drawing messages if we hit an empty one
    if (strlen(message.message) == 0 && strlen(message.name) == 0) {
      break;
    }

    //Determine if the the mesage is two line or not
    bool twoLine = (strlen(message.name) + strlen(message.message) > _maxCharsPerRow);

    //Move cursor up to print the line(s)
    uint8_t y = display.getCursorY() - ANX_FONT_HEIGHT;
    if (twoLine) {
      y -= ANX_FONT_HEIGHT;
    }
    display.setCursor(0, y);

    //Ensure username is null terminated
    char name[USERNAME_MAX_LENGTH + 1];
    memcpy(name, message.name, USERNAME_MAX_LENGTH);
    name[USERNAME_MAX_LENGTH] = '\0';

    //Ensure message is null terminated
    char msg[ANX_CHAT_MAX_LENGTH + 1];
    memcpy(msg, message.message, ANX_CHAT_MAX_LENGTH);
    msg[ANX_CHAT_MAX_LENGTH] = '\0';

    display.print("@");
    display.print(name);
    display.print(":");
    display.println(msg);

    //Move cursor back up above the message we just printed
    display.setCursor(0, y);

    row--;
  }

  //Show
  safeDisplay();
}


