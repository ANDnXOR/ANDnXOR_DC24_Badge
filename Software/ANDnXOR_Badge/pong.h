#ifndef PONG_H
#define PONG_H
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>

#define BALL_RADIUS     3

#define PADDLE_HEIGHT   15
#define PADDLE_WIDTH    3
#define PADDLE_RADIUS   2
#define PADDLE_SPEED    2.8
#define BALL_VELOCITY   4.8
#define BALL_VY_BIAS    4.0
#define AI_MIN_DELAY    80
#define PONG_STEP       80
#define PONG_DEFLECT_XP 7
#define PONG_WIN_XP     10

struct pongState {
  int16_t player1Y, player2Y;
  int16_t ballX, ballY;
  float ballVX, ballVY;
  int32_t lastAIMove;
  uint32_t xp;
};

extern Adafruit_SSD1306 display;
extern void doPong();

#endif
