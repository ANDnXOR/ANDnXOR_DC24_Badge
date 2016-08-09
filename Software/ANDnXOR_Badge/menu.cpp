#include <Adafruit_GFX.h>
#include "ANX.h"
#include "anim.h"
#include "buttons.h"
#include "chat.h"
#include "games.h"
#include "graphics.h"
#include "life.h"
#include "menu.h"
#include "ninja.h"
#include "pong.h"
#include "rf.h"
#include "serial.h"
#include "settings.h"
#include "strings.h"

extern ANDnXORMenu menu;

ANDnXORMenu::ANDnXORMenu() {
  _count = 0;
  _top = 0;
  _selected = 0;
  _maxRows = (display.height() - ANX_FONT_HEIGHT - 3) / ANX_FONT_HEIGHT;
  _title = "AND!XOR";
}

void ANDnXORMenu::setMenu(menuItem *items, uint8_t count) {
  _items = items;
  _count = count;
}

void ANDnXORMenu::clearMenu() {
  _count = 0;
  _top = 0;
  _selected = 0;
  display.clearDisplay();
}

void ANDnXORMenu::scrollDown() {
  _top++;
  if (_top + _maxRows > _count) _top--;
  this->_drawMenu();
}

void ANDnXORMenu::scrollUp() {
  _top--;
  if (_top < 0) _top = 0;
  this->_drawMenu();
}

void ANDnXORMenu::goToTop() {
  _top = 0;
  _selected = 0;
}

/**
   Main entry point to the menu. Menu must be setup prior. This will draw then block on user entry
*/
int8_t ANDnXORMenu::doMenu() {
  ledsOff();
  bool changed = true;

  //Button counter for mortal combat mode
  uint8_t button_code = 0;

  while (true) {

    //Only redraw the menu if something has changed
    if (changed) {
      this->_drawMenu();
      changed = false;
    }

    uint32_t eyes = roygbiv[5];
    if (ANXGetLevel() > 40) {
      eyes = roygbiv[0];
    } else if (ANXGetLevel() >= 30) {
      eyes = roygbiv[1];
    } else if (ANXGetLevel() >= 20) {
      eyes = roygbiv[2];
    } else if (ANXGetLevel() >= 10) {
      eyes = roygbiv[3];
    }
    setEyeLeftLed(eyes);
    setEyeRightLed(eyes);

    uint8_t button = getButtonState();

    //If we're going to move up regardless of input, then move
    if ((button & BUTTON_UP) > 0) {
      _selected--;
      if (_selected <= 0) _selected = 0;
      if (_selected < _top) {
        scrollUp();
      }
      changed = true;
      deepSleep(BUTTON_REPEAT_DELAY);
      if (button_code == 0 || button_code == 1) {
        button_code++;
      } else {
        button_code = 0;
      }
    }

    //Down moves (and possibly scrolls) the selected item down
    if ((button & BUTTON_DOWN) > 0) {
      _selected++;
      if (_selected >= _count) _selected = _count - 1;
      if (_selected >= (_top + _maxRows)) {
        scrollDown();
      }
      changed = true;
      deepSleep(BUTTON_REPEAT_DELAY);
      if (button_code == 2 || button_code == 3) {
        button_code++;
      } else {
        button_code = 0;
      }
    }

    //Left button should return back status
    if ((button & BUTTON_LEFT) > 0) {
      //Don't return back if doing secret code
      if (button_code == 4 || button_code == 6) {
        button_code++;
        button = 0;
        clearButtonState();
      }
      //Normal handling
      else {
        button_code = 0;
        changed = true;
        clearButtonState();
        return MENU_BACK;
      }
    }

    //Enter button returns the selected index
    if ((button & BUTTON_ENTER) > 0) {
      if (button_code == 8) {
        //They have successfully coded in
        //U,U,D,D,L,R,L,R, Enter
        clearButtonState();
        //Enable XP per level to 80.
        statusDialog("!!MORTAL KOMBAT!!");
        safeWaitForButton();
        XP_PER_LEVEL = 80;
        button = 0;
      } else {
        clearButtonState(); //Don't allow repeats
        return _selected;
      }
    }

    //Only used if we're listening for mortal kombat mode
    if ((button & BUTTON_RIGHT) > 0) {
      if (button_code == 5 || button_code == 7) {
        button_code++;
      } else {
        button_code = 0;
      }

      //Force menu to redraw
      changed = true;

      //clear current button press
      button = 0;
      clearButtonState();
    }

    //If no button was pressed sleep for a bit
    if (button == 0) {
      deepSleepInterruptable(250);
      tick();
    }
  }
}

/**
   Draw the menu in its current state
*/
void ANDnXORMenu::_drawMenu() {
  uint8_t titleBarHeight = ANX_FONT_HEIGHT + 3;
  uint8_t menuHeight = display.height() - titleBarHeight;

  //Setup the font
  display.clearDisplay();
  display.setCursor(0, 0);

  //Draw top bar
  char username[USERNAME_MAX_LENGTH + 1];
  memset(username, '\0', USERNAME_MAX_LENGTH + 1); //clear username memory to null. add an extra null at the end to ensure null termination
  ANXGetUsername(username);
  display.setCursor(1, 1);
  display.print(_title);
  printAlignRight(username, 1);
  display.fillRect(0, 0, display.width(), ANX_FONT_HEIGHT + 2, INVERSE);

  //Draw airplane mode if necessary
  if (ANXGetAirplane()) {
    drawBitmapFlash(getBitmapMetadata(AIRPLANE_address), 53, 1, INVERSE);
  }
  //Otherwise show user's level
  else {
    display.setCursor(53, 1);
    display.print(ANXGetLevel());
  }

  if (_count > 0) {
    //Draw each menu item
    display.setCursor(0, titleBarHeight);
    uint8_t t = _top; //temporary "top" index
    for (uint8_t i = 0; i < _maxRows; i++) {
      //Invert display if selected
      if (t == _selected) {
        display.fillRoundRect(0, (i * ANX_FONT_HEIGHT) + titleBarHeight, display.width() - ANDnXORMenu_SCROLL_WIDTH - 2, ANX_FONT_HEIGHT, 2, WHITE);
      }
      display.print(t + 1);
      display.print(". ");
      display.println(_items[t].text);
      t++;
      if (t >= _count) break;
    }

    //Draw the scrollbar
    if (_count > _maxRows) {
      int x = display.width() - ANDnXORMenu_SCROLL_WIDTH - 1;
      int h = menuHeight * ((float)_maxRows / (float)_count);
      int y = (int)((float)(menuHeight - h) * (float)_top / (float)(_count - _maxRows + 1)    ) +  titleBarHeight;
      display.fillRoundRect(x, y, ANDnXORMenu_SCROLL_WIDTH, h, ANDnXORMenu_SCROLL_WIDTH / 2, WHITE);
    }
  }

  //Draw the screen
  safeDisplay();
}

/**
   Sets the title of the menu
*/
void ANDnXORMenu::setTitle(char *title) {
  _title = title;
}

void airplaneModeMenuItem() {
  ANXSetAirplane(yesNoDialog("Enable Airplane\nMode?"));
}

void alertsMenuItem() {
  ANXSetAlert(yesNoDialog("Enable Alerts?"));
}

void eyesMenuItem() {
  ANXSetEyes(yesNoDialog("Enable LED Eyes?"));
}

void partyAnimationMenuItem() {
  party(100);
}

void ragerAnimationMenuItem() {
  party(0);
}

void screenRotationMenuItem() {
  ANXSetTilt(yesNoDialog("Enable Screen\nRotation?"));
}

/**
  Handle animation menu. Keep the user here until they quit.
*/
void animationMenu() {
  uint8_t count = 0;
  uint8_t unlock = ANXGetUnlocked();

  menuItem items[32];

  items[count++] = {mi_anim_toaster, &flyingToasters};
  items[count++] = {mi_anim_knight, &knightRider};
  items[count++] = {mi_anim_matrix, &matrix};
  items[count++] = {mi_anim_party, &partyAnimationMenuItem};
  items[count++] = {mi_anim_snake, &snake};
  items[count++] = {mi_anim_glow, &glow};
  items[count++] = {mi_anim_rainbow, &rainbow};
  items[count++] = {mi_anim_netscape, &netscape};
  items[count++] = {mi_anim_scroll, &scrollingText};
  items[count++] = {mi_anim_nayan, &nayan};
  items[count++] = {mi_anim_defcon, &defcon};
  items[count++] = {mi_anim_hackers, &hackers};
  items[count++] = {mi_anim_flames, &flames};
  items[count++] = {mi_anim_wargames, &warGames};
  items[count++] = {mi_anim_game, &gameOfLife};

  //AND!XOR unlocked
  if ((unlock & UNLOCK_ANDNXOR) > 0) {
    items[count++] = {mi_anim_major, &majorLazer};
    items[count++] = {mi_anim_rager, &ragerAnimationMenuItem};
  }

  //EFF Mode unlocked
  if ((unlock & UNLOCK_EFF) > 0) {
    items[count++] = {mi_anim_eff, &eff};
  }

  //Hackaday
  if ((unlock & UNLOCK_HACKADAY) > 0) {
    items[count++] = {mi_anim_hackaday, &hackaday};
  }

  //Master badge unlocked
  if ((unlock & UNLOCK_MASTER) > 0) {
    items[count++] = {mi_anim_rick, &rick};
  }

  //Whiskey Pirate Unlocked
  if ((unlock & UNLOCK_PIRATES) > 0) {
    items[count++] = {mi_anim_pirate, &pirate};
  }

  //About scroll unlocked
  if ((unlock & UNLOCK_SCROLL) > 0) {
    items[count++] = {mi_anim_lycos, &lycos};
    items[count++] = {mi_anim_cyber, &cyberPathogen};
  }

  //Whoami unlocked
  if ((unlock & UNLOCK_WHOAMI) > 0) {
    items[count++] = {mi_anim_remember, &rememberme};
  }

  menu.setMenu(items, count);
  menu.setTitle("Bling");
  menu.goToTop();

  while (1) {
    int8_t selected = menu.doMenu();
    if (selected == MENU_BACK)  {
      return;
    }
    items[selected].callback();
  }
}

/**
   Sub menu for chat with options for quick chat
*/
void chatMenu() {
  menuItem items[] = {
    {mi_main_chat, &doChat},
    {mi_main_qchat1, &doQuickChat1},
    {mi_main_qchat2, &doQuickChat2},
    {mi_main_qchat3, &doQuickChat3},
    {mi_main_qchat4, &doQuickChat4},
    {mi_main_qchat5, &doQuickChat5},
  };

  menu.setMenu(items, 6);
  menu.setTitle(mi_main_chat);
  menu.goToTop();

  while (1) {
    int8_t selected = menu.doMenu();
    if (selected == MENU_BACK) return;
    items[selected].callback();
  }
}
/**
   Handle game menu. Keep the user here until they quit.
*/
void gameMenu() {
  menuItem items[] = {
    {mi_game_progress, &gameProgress},
    {mi_game_ninja, &ninja},
    {mi_game_pong, &doPong},
    {mi_game_flappy, &flappy},
    {mi_game_asteroids, &asteroids},
    {mi_game_ski, &ski},
    {mi_game_dodge, &dodge}
  };


  while (1) {
    menu.setMenu(items, sizeof(items) / sizeof(menuItem));
    menu.setTitle("Games");
    menu.goToTop();
    int8_t selected = menu.doMenu();
    if (selected == MENU_BACK) return;
    items[selected].callback();
  }
}

/**
  Main menu
*/
void mainMenu() {
  uint8_t count = 0;
  menuItem items[10];
  items[count++] = {mi_main_anim, &animationMenu};
  items[count++] = {mi_main_chat, &chatMenu};
  items[count++] = {mi_main_games, &gameMenu};
  items[count++] = {mi_main_peers, &peersMenu};
  items[count++] = {mi_main_system, &systemMenu};

#ifdef MASTER
  items[count++] = {mi_main_alert, &sendAlert};
  items[count++] = {mi_main_troll, &troll};
#endif

  while (1) {
    menu.setMenu(items, count);
    menu.setTitle("AND!XOR");
    menu.goToTop();

    int8_t selected = menu.doMenu();

    //Show unlock status leds if they try to exit from menu
    if (selected == MENU_BACK) {
      unlockStatus();
    }

    if (selected >= 0) {
      items[selected].callback();
    }
  }
}

/**
   Show the system menu and handle selections
*/
void systemMenu() {
  uint8_t unlock = ANXGetUnlocked();
  uint8_t count = 0;
  menuItem items[10];

  items[count++] = {mi_set_name, &editUsername};
  items[count++] = {mi_set_airplane, &airplaneModeMenuItem};
  items[count++] = {mi_set_screen, &screenRotationMenuItem};

  //AND!XOR unlocked
  if ((unlock & UNLOCK_ANDNXOR) > 0) {
    items[count++] = {mi_set_eye, &eyesMenuItem};
    items[count++] = {mi_set_alert, &alertsMenuItem};
    items[count++] = {mi_set_spectrum, &doSpectrumAnalyzer};
  }

  items[count++] = {mi_set_rf, &doRFDebug};
  items[count++] = {mi_set_test, &selfTest};
  items[count++] = {mi_set_serial, &doSerial};
  items[count++] = {mi_set_about, &about};

  menu.setMenu(items, count);
  menu.setTitle("System");
  menu.goToTop();

  while (1) {
    int8_t selected = menu.doMenu();
    if (selected == MENU_BACK) return;
    items[selected].callback();
  }
}

/**
   Show peer details
*/
static void peerDetails(uint8_t nodeid) {
  PeerNode peer = peers[nodeid];
  uint8_t button = getButtonState();
  char lastSeen[20];
  memset(lastSeen, '\0', 20);

  while ((button & BUTTON_LEFT) == 0) {
    uint32_t lastSeenMillis = rtMillis() - peer.lastSeen;
    if (lastSeenMillis < 2000) {
      sprintf(lastSeen, " A second ago");
    } else if (lastSeenMillis < 60 * 1000) {
      sprintf(lastSeen, " %d seconds ago", lastSeenMillis / 1000);
    } else if (lastSeenMillis < 2 * 60 * 1000) {
      sprintf(lastSeen, " A minute ago");
    } else {
      sprintf(lastSeen, " %d minutes ago", lastSeenMillis / 60 / 1000);
    }

    window(peer.name);
    display.setTextWrap(true);
    display.setCursor(0, 10);
    display.print("Level: "); display.println(peer.level);
    display.print("Last Seen: ");
    display.println(lastSeen);
    display.print("RSSI: "); display.print(peer.rssi); display.println("dBm");
    display.setTextWrap(false);
    display.println("[ENTER] --> Ping");

#ifdef MASTER
    display.println("U=LVL D=UNLK R=TRL");
#endif
    safeDisplay();
    button = waitForButton();

    if ((button & BUTTON_ENTER) > 0) {
      int t = ANXRFSendPing(peer.nodeid);
      char ping[20];
      if (t >= 0) {
        sprintf(ping, "Round Trip=\n%d ms", t);
      } else {
        sprintf(ping, "No Response");
      }
      statusDialog(ping);
      safeWaitForButton();
    }

#ifdef MASTER
    if ((button & BUTTON_DOWN) > 0) {
      ANXRFSendUnlock(peer.nodeid);
    } else if ((button & BUTTON_UP) > 0) {
      ANXRFSendLevelUp(peer.nodeid);
    } else if ((button & BUTTON_RIGHT) > 0) {
      ANXRFSendTrollRick(peer.nodeid);
    }
#endif
  }
}

/**
   Helper function that displays a menu of peers and returns the selected nodeid
*/
int16_t getPeerFromUser() {
  menuItem m_peers[PEER_NODES_MAX];

  //Array of pointers to peers, as they're added to the menu they are put here in the
  //same order such that when the menu returns a selected index we can get back to the
  //PeerNode that was selected
  PeerNode* menuPeers[PEER_NODES_MAX];

  //Build the menu with just recent peers
  uint8_t count = 0;
  for (uint8_t i = 0; i < PEER_NODES_MAX; i++) {
    if ((rtMillis() - peers[i].lastSeen) <= MAX_PEER_AGE && peers[i].lastSeen > 0) {
      //Store a pointer for later
      menuPeers[count] = &peers[i];
      count++;
    }
  }

  //Bubble sort, YOLO
  for (uint8_t i = 0; i < count; i++) {
    for (uint8_t j = i + 1; j < count; j++) {
      if (menuPeers[i]->rssi < menuPeers[j]->rssi) {
        //swap
        PeerNode *temp = menuPeers[j];
        menuPeers[j] = menuPeers[i];
        menuPeers[i] = temp;
      }
    }
  }

  //Generate the menu items for the peers
  for (uint8_t i = 0; i < count; i++) {
    //Create the menu item
    m_peers[i].text = (char *) malloc(18);
    sprintf(m_peers[i].text, "%s [%d]", menuPeers[i]->name, menuPeers[i]->level);
  }

  //Display the menu
  menu.setMenu(m_peers, count);
  menu.setTitle("Peers");
  menu.goToTop();
  int8_t selected =  menu.doMenu();

  //Now that the menu has been displayed free the memory used by the rows
  for (uint8_t i = 0; i < count; i++) {
    free(m_peers[i].text);
  }

  if (selected == MENU_BACK) {
    return -1;
  }

  return menuPeers[selected]->nodeid;
}

/**
   Handle peers menu
*/
void peersMenu() {
  while (1) {
    int16_t nodeid = getPeerFromUser();
    if (nodeid == -1) return;
    //Show details on the selected peer
    peerDetails(nodeid);
  }
}
