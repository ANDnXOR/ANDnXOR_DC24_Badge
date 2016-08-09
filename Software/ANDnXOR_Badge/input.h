#ifndef ANXINPUT_H
#define ANXINPUT_H

void ANXInputClearChar(uint8_t row, uint8_t col, uint8_t startX, uint8_t startY);
void ANXInputDrawCursor(uint8_t row, uint8_t col, uint8_t startX, uint8_t startY);
void ANXInputMoveCursor(uint8_t row, uint8_t col, uint8_t startX, uint8_t startY);
void ANXInput(char *message, uint8_t startX, uint8_t startY, uint8_t maxChars = ANX_INPUT_MAX, uint8_t maxCharsPerRow = 0);
void ANXInputWindow(char *message, char * title, uint8_t maxChars);

#endif
