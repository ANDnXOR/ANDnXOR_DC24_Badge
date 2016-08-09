#ifndef ANX_BUTTONS_H
#define ANX_BUTTONS_H

#include <Arduino.h>

#define BUTTON_UP       B00000001
#define BUTTON_DOWN     B00000010
#define BUTTON_LEFT     B00000100
#define BUTTON_RIGHT    B00001000
#define BUTTON_ENTER    B00010000

#define DEBOUNCE_DELAY        5

extern uint8_t _upButton;
extern uint8_t _downButton;
extern uint8_t _leftButton;
extern volatile uint8_t _buttonState;

void buttonHandler(uint8_t button, uint8_t mask);
void onDown();
void onEnter();
void onLeft();
void onRight();
void onUp();
void clearButtonState();
uint8_t getButtonState();
void resetDownTimes();
void safeClearButtonState();
void safeWaitForButton();
void setupButtons();
void tiltHandler(uint8_t button);
uint8_t waitForButton();

#endif
