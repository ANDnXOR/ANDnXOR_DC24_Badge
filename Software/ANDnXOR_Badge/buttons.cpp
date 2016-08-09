#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <libmaple/gpio.h>
#include "buttons.h"
#include "rf.h"
#include "serial.h"

#define BUTTON_UP_PIN               PC13
#define BUTTON_DOWN_PIN             PC14
#define BUTTON_LEFT_PIN             PC15
#define BUTTON_ENTER_PIN            PB8
#define BUTTON_RIGHT_PIN            PB5
volatile uint8_t _buttonState = 0;

extern Adafruit_SSD1306   display;
/**
 * Setup button pins
 */
void setupButtons() {

  //Configure internal pull downs on the user input pins
  gpio_set_mode(GPIOC, 13, GPIO_INPUT_PU);
  gpio_set_mode(GPIOC, 14, GPIO_INPUT_PU);
  gpio_set_mode(GPIOC, 15, GPIO_INPUT_PU);
  gpio_set_mode(GPIOB, 5, GPIO_INPUT_PU);
  gpio_set_mode(GPIOB, 8, GPIO_INPUT_PU);
  gpio_set_mode(GPIOA, 15, GPIO_INPUT_PU);

  attachInterrupt(BUTTON_UP_PIN, onUp, CHANGE);
  attachInterrupt(BUTTON_DOWN_PIN, onDown, CHANGE);
  attachInterrupt(BUTTON_RIGHT_PIN, onRight, CHANGE);
  attachInterrupt(BUTTON_ENTER_PIN, onEnter, CHANGE);
  attachInterrupt(BUTTON_LEFT_PIN, onLeft, CHANGE);  //Left and tilt share interrupt
}

/**
 * Generic button handler that handles debouncing and holding down
 * returns true if handled, false otherwise
 */
void buttonHandler(uint8_t button, uint8_t mask) {
  noInterrupts();
  //If the button comes into the handler low, delay to remove bounces then check again
  if (digitalRead(button) == LOW) {
    rtDelay(DEBOUNCE_DELAY);
    //If button is still low, it is not erroneous, set the bit otherwise unset it.
    if (digitalRead(button) == LOW) {
      _buttonState |= mask;
    } else {
      _buttonState &= ~mask;
    }
  }
  //If the button comes into the handler high, delay to remove bounces then check again
  else if (digitalRead(button) == HIGH) {
    rtDelay(DEBOUNCE_DELAY);

    //really is low unset the bit
    if (digitalRead(button) == HIGH) {
      _buttonState &= ~mask;
    }
  }

  interrupts();
}

/**
 * Handler for rising edge of down button, responsible for debouncing and setting button state
 */
void onDown() {
  if (displayInverted) buttonHandler(BUTTON_DOWN_PIN, BUTTON_UP);
  else buttonHandler(BUTTON_DOWN_PIN, BUTTON_DOWN);
}


/**
 * Handler for rising edge of left button, responsible for debouncing and setting button state
 * Left shares an interrupt with tilt switch - debouce should eliminate it
 */
void onLeft() {
  if (displayInverted) buttonHandler(BUTTON_LEFT_PIN, BUTTON_RIGHT);
  else buttonHandler(BUTTON_LEFT_PIN, BUTTON_LEFT);
}

/**
 * Handler for rising edge of up button, responsible for debouncing and setting button state
 */
void onUp() {
  if (displayInverted) buttonHandler(BUTTON_UP_PIN, BUTTON_DOWN);
  else buttonHandler(BUTTON_UP_PIN, BUTTON_UP);
}

/**
 * Handler for rising edge of right button, responsible for debouncing and setting button state
 */
void onRight() {
  if (displayInverted) buttonHandler(BUTTON_RIGHT_PIN, BUTTON_LEFT);
  else buttonHandler(BUTTON_RIGHT_PIN, BUTTON_RIGHT);
}

/**
 * Handler for rising edge of enter button, responsible for debouncing and setting button state
 */
void onEnter() {
  buttonHandler(BUTTON_ENTER_PIN, BUTTON_ENTER);
}

uint8_t getButtonState() {
  return _buttonState;
}

void clearButtonState() {
  _buttonState = 0;
}

/**
 * Clears the button state but in a way that avoids accidental repitions of the button press.
 */
void safeClearButtonState() {
  deepSleep(500);
  clearButtonState();
}

/**
 * This should be used to safely wait for button press by the user while avoiding accidental
 * repetitions of the button press. Useful when exiting menus or games.
 */
void safeWaitForButton() {
  waitForButton();
  safeClearButtonState();
}

/**
 * Blocks for any button input. Once occurs, clears state, and returns the button results
 */
uint8_t waitForButton() {
  while (_buttonState == 0) {
    deepSleepInterruptable(500);
    //Handle tasks
    tick();
  }
  uint8_t button = _buttonState;
  return button;
}


