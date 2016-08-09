#ifndef GAMES_H
#define GAMES_H

#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include "flash.h"
#include "games.h"

//Asteroids Settings
#define ASTEROIDS_BULLET_STEP_MS  300
#define ASTEROIDS_GAME_STEP_MS    100
#define ASTEROIDS_NUMBER_BULLETS  8
#define ASTEROIDS_NUMBER_SKULLS   4
#define ASTEROIDS_MAX_DX          1.6
#define ASTEROIDS_MAX_DY          1.6
#define ASTEROIDS_BULLET_VELOCITY 3.5
#define ASTEROIDS_SHIP_ACCEL      0.7
#define ASTEROIDS_SHIP_MARGIN     4
#define ASTEROIDS_XP_PER_SKULL    3

//Dodge settings
#define DODGE_CREATE_OBJECT_PCT 10
#define DODGE_FALL_VELOCITY     1.9
#define DODGE_STEP_MS           80
#define DODGE_PLAYER_STEP_MS    100
#define DODGE_SPRITE_COUNT      16
#define DODGE_POINTS_PER_FLOPPY 10
#define DODGE_POINTS_PER_MISS   -8
#define DODGE_XP_PER_POINT      0.3

//Flappy settings
#define FLAPPY_PIPE_COUT        4
#define FLAPPY_PIPE_GAP         30
#define FLAPPY_PIPE_MIN_HEIGHT  8
#define FLAPPY_PIPE_SPACING     90
#define FLAPPY_PLAYER_X         30
#define FLAPPY_PIPE_MAX_HEIGHT  64 - PIPE_GAP - (2 * PIPE_MIN_HEIGHT)
#define FLAPPY_PIPE_VELOCITY    3
#define FLAPPY_UP_TIME_MS       500
#define FLAPPY_STEP_MS          80
#define FLAPPY_MAX_VELOCITY     8
#define FLAPPY_VELOCITY_UP      -1.5
#define FLAPPY_GRAVITY          0.8

//Ski settings
#define SKI_ACCELERATION  0.01
#define SKI_M_PER_XP      70
#define SKI_STEP_MS       80
#define SKI_X             56
#define SKI_Y             4
#define SKI_MARGIN        4
#define SKI_SPRITE_COUNT  16
#define SKI_WORLD_WIDTH   3
#define SKI_WORLD_HEIGHT  2

extern Adafruit_SSD1306   display;
extern ANXFlash           flash;
extern Adafruit_NeoPixel  leds;

struct sprite {
  float x, y;
  int8_t dx, dy;
  uint32_t address;
};

struct bullet {
  float x, y, dx, dy;
};

struct dodgeState {
  int32_t points;
  float x;
  sprite sprites[DODGE_SPRITE_COUNT];
};

struct pipe {
  int16_t x;
  uint8_t h;
  bool passed;
};

struct flappyState {
  bool flapUp;
  float playerY;
  pipe pipes[FLAPPY_PIPE_COUT];
  uint16_t pipesPassed;
  uint32_t xp;

  uint8_t skullSizeBytes = 26;
  uint8_t skullDown[26];
  uint8_t skullUp[26];

  uint8_t pipeSizeBytes = 120;
  uint8_t pipeBottom[120];
  uint8_t pipeTop[120];
};

struct skiState {
  sprite sprites[SKI_SPRITE_COUNT];
  int8_t angle;
  float distance;
  float velocity, velocityAngled;
};

struct skull {
  float x, y, dx, dy;
};

extern void asteroids();
extern void dodge();
extern void flappy();
extern void gameProgress();
extern void ski();

#endif
