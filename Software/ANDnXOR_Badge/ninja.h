#ifndef NINJA_H
#define NINJA_H

#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>

#include "ANX.h"
#include "buttons.h"
#include "graphics.h"
#include "menu.h"
#include "rf.h"
#include "serial.h"

#define PLAYER_1_X        10
#define PLAYER_1_Y        2
#define PLAYER_2_X        91
#define PLAYER_2_Y        2
#define STATE_SELECT_MOVE 1
#define STATE_FIGHT       2
#define STATE_ABORT       99
#define MOVE_PUNCH        0
#define MOVE_KICK         1
#define MOVE_SHIELD       2
#define MOVE_NONE         3
#define MOVE_TROLL        4
#define RESULT_WIN        1
#define RESULT_LOSE       2
#define RESULT_TIE        3
#define TYPE_INIT         0
#define TYPE_ACK          1
#define TYPE_NACK         2
#define TYPE_MOVE         3


extern Adafruit_SSD1306   display;
extern Adafruit_NeoPixel  leds;

struct NinjaState {
  bitmap p1Idle1, p2Idle1;
  bitmap p1Idle2, p2Idle2;
  bitmap p1Punch1, p2Punch1;
  bitmap p1Punch2, p2Punch2;
  bitmap p1Kick1, p2Kick1;
  bitmap p1Kick2, p2Kick2;
  bitmap p1Shield1, p2Shield1;
  bitmap p1Shield2, p2Shield2;
  bitmap p1Dead, p2Dead;
  bitmap troll;
  bitmap punch, kick, shield;
  bitmap up, down, right;
  uint8_t state;
  int8_t p1Move, p2Move;
  int8_t lastACK;          //Should be set to -1 (no ACK) or the type of packet acknowledgeds
  int8_t lastNACK;         //Similar to ACK
  uint8_t round;
  uint8_t p1Score, p2Score;
  uint8_t p2nodeid;
  uint8_t p2Level; 
  char *p2Name;
  bool tiebreaker[3];
};

extern void ninja();
extern void ninjaChallenged(NinjaPacket packet);

#endif
