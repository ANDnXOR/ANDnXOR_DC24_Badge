#ifndef ANDnXORMenu_h
#define ANDnXORMenu_h

#include "Arduino.h"
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <Adafruit_GFX.h>
#include "buttons.h"
#include "rf.h"

#define ANDnXORMenu_SCROLL_WIDTH 3

extern Adafruit_SSD1306 display;

struct menuItem {
  char *text;
  void (*callback)(void);
};

class ANDnXORMenu {
  public:
    ANDnXORMenu();
    void setMenu(menuItem *items, uint8_t count);
    void setTitle(char *title);
    void clearMenu();
    int8_t doMenu();
    void scrollDown();
    void scrollUp();
    void goToTop();
  private:
    uint8_t _count;
    uint8_t _top;
    int8_t _selected; //needs to be signed to prevent underrun of menu
    uint8_t _maxRows;
    menuItem *_items;
    char* _title;
    void _drawMenu();
};

extern void animationMenu();
extern void chatMenu();
extern void gameMenu();
int16_t getPeerFromUser();
extern void mainMenu();
extern void peersMenu();
extern void systemMenu();
extern void troll();

#endif
