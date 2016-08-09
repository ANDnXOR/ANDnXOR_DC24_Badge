#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include "ANX.h"
#include "buttons.h"
#include "input.h"
#include "rf.h"

extern Adafruit_SSD1306 display;

void ANXInputClearChar(uint8_t row, uint8_t col, uint8_t startX, uint8_t startY) {
  display.fillRect(startX + (col * ANX_FONT_WIDTH), startY + (row * ANX_FONT_HEIGHT), ANX_FONT_WIDTH, ANX_FONT_HEIGHT, BLACK);
}

void ANXInputDrawCursor(uint8_t row, uint8_t col, uint8_t startX, uint8_t startY) {
  display.fillRect(startX + (col * ANX_FONT_WIDTH), startY + (row * ANX_FONT_HEIGHT), ANX_FONT_WIDTH, ANX_FONT_HEIGHT, INVERSE);
}

void ANXInputMoveCursor(uint8_t row, uint8_t col, uint8_t startX, uint8_t startY) {
  display.setCursor(startX + (col * ANX_FONT_WIDTH), (row * ANX_FONT_HEIGHT) + startY);
}

/**
   Interactively accepts input from the user
   Max input size is determined by remaining cells at given start row. No scrolling.
*/
void ANXInput(char *message, uint8_t startX, uint8_t startY, uint8_t maxChars, uint8_t maxCharsPerRow) {
  int messageLen = strlen(message);             //Size of the message
  int cursor = messageLen;       //Location of cursor in the text, put cursor at end of message to continue
  uint8_t row = 0;          //Row of the cursor
  uint8_t col = 0;          //Col of the cursor
  uint8_t cindex = 0;   //index within input char array
  int maxCols = (display.width() - startX) / ANX_FONT_WIDTH;    //Max number of cols available
  int maxRows = (display.height() - startY) / ANX_FONT_HEIGHT;  //Max number of rows available
  int maxLen = min(maxCols * maxRows, maxChars);           //Maximum possible size of the message (limited by input space and buffer)

  //Use the user specified chars per row if specified
  if (maxCharsPerRow > 0) {
    maxCols = min(maxCharsPerRow, maxCols);
  }

  //Print the message, if any
  ANXInputMoveCursor(row, col, startX, startY);
  display.print(message);

  //Iteractively get input from user
  while (1) {
    row = cursor / maxCols;
    col = cursor % maxCols;

    ANXInputClearChar(row, col, startX, startY);
    ANXInputDrawCursor(row, col, startX, startY);

    if (message[cursor] != NULL) {
      ANXInputMoveCursor(row, col, startX, startY);
      display.print(message[cursor]);
    }

    safeDisplay();

    uint8_t button = waitForButton();
    if ((button & BUTTON_DOWN) > 0) {

      //Move char index to the end pro-actively since unsigned won't wrap to -1
      if (cindex == 0) cindex = INPUT_CHARS_COUNT;
      cindex--;

      if (cursor >= messageLen) {
        messageLen++;
      }
      message[cursor] = INPUT_CHARS[cindex];

      //Make sure we're not null terminating the string early
      if (cursor > 0) {
        if (message[cursor - 1] == '\0') message[cursor - 1] = ' ';
      }

      deepSleep(BUTTON_REPEAT_DELAY);
    }

    if ((button & BUTTON_UP) > 0) {
      cindex++;
      if (cindex >= INPUT_CHARS_COUNT) cindex = 0;
      if (cursor >= messageLen) {
        messageLen++;
      }
      message[cursor] = INPUT_CHARS[cindex];

      //Make sure we're not null terminating the string early
      if (cursor > 0) {
        if (message[cursor - 1] == '\0') message[cursor - 1] = ' ';
      }

      deepSleep(BUTTON_REPEAT_DELAY);
    }

    if ((button & BUTTON_RIGHT) > 0) {
      ANXInputDrawCursor(row, col, startX, startY);

      //Prevent cursor from going off the deep end
      if (cursor < maxLen - 1) {
        cursor++;
      }

      if (cursor < messageLen) {
        cindex = ANXIndexOf(INPUT_CHARS, message[cursor]);
      } else {
        messageLen++;
        cindex = 0;
      }

      //Make sure we're not null terminating the string early
      if (cursor > 0) {
        if (message[cursor - 1] == '\0') message[cursor - 1] = ' ';
      }

      //Don't allow repeats
      clearButtonState();
    }

    if ((button & BUTTON_LEFT) > 0) {
      //Prevent cursor underun
      if (cursor > 0) {
        ANXInputDrawCursor(row, col, startX, startY);
        cursor--;
      }

      if (cursor < messageLen) {
        cindex = ANXIndexOf(INPUT_CHARS, message[cursor]);
      } else {
        messageLen++;
        cindex = 0;
      }

      //Don't allow repeats
      clearButtonState();
    }

    if ((button & BUTTON_ENTER) > 0) {
      //Don't allow repeats
      clearButtonState();
      return;
    }
  }
}

/**
   Popup input window with text centered. Window height is based on maxChars
*/
void ANXInputWindow(char * message, char *title, uint8_t maxChars) {

  uint8_t w = (display.width() * .8) + 4;
  uint8_t maxCharsPerRow = w / ANX_FONT_WIDTH;
  uint8_t rows = (maxChars / maxCharsPerRow) + 1;
  uint8_t textHeight = rows * ANX_FONT_HEIGHT;
  uint8_t h = textHeight + ANX_FONT_HEIGHT + 3 + 2;
  uint8_t x = (display.width() - w) / 2;
  uint8_t y = (display.height() - h) / 2;
  uint8_t tx = (display.width() - (ANX_FONT_WIDTH * strlen(title))) / 2;
  uint8_t ty = y + 1;

  display.fillRoundRect(x, y, w, h, 2, BLACK);
  display.drawRoundRect(x, y, w, h, 2, WHITE);
  display.fillRoundRect(x, y, w, ANX_FONT_HEIGHT + 1, 2, WHITE);

  display.setCursor(tx, ty);
  display.setTextColor(INVERSE);
  display.print(title);

  ANXInput(message, x + 2, y + ANX_FONT_HEIGHT + 3, maxChars, maxCharsPerRow);
}

