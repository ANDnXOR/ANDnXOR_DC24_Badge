#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <Arduino.h>

#include "ANX.h"
#include "buttons.h"
#include "life.h"

uint8_t exploder[4][3] = {
  {0, 1, 0},
  {1, 1, 1},
  {1, 0, 1},
  {0, 1, 0}
};
uint8_t glider[3][3] = {
  {0, 1, 0},
  {0, 0, 1},
  {1, 1, 1}
};

uint8_t spaceship[4][5] = {
  {0, 1, 1, 1, 1},
  {1, 0, 0, 0, 1},
  {0, 0, 0, 0, 1},
  {1, 0, 0, 1, 0}
};

extern Adafruit_SSD1306 display;

/**
   Gets boolean value at specific x,y coordinates
*/
bool gameOfLifeGet(uint8_t *life, uint8_t x, uint8_t y) {
  uint16_t offset = x + (y * ANX_LIFE_WIDTH);
  uint8_t block = life[offset / 8];
  return bitRead(block, offset % 8);
}

/**
   Sets boolean value at specific x,y coordinates
*/
void gameOfLifeSet(uint8_t *bits, uint8_t x, uint8_t y, bool value) {
  uint16_t offset = x + (y * ANX_LIFE_WIDTH);
  uint8_t block = bits[offset / 8];
  bitWrite(block, offset % 8, value);
  bits[offset / 8] = block;
}

/**
   Draw the pixels on the game of life board
*/
void drawGameOfLife(uint8_t *life) {
  display.clearDisplay();
  for (int y = 0; y < ANX_LIFE_HEIGHT ; y++) {
    for (int x = 0; x < ANX_LIFE_WIDTH; x++) {
      if (gameOfLifeGet(life, x, y)) {
        display.drawPixel(x, y, WHITE);
      }
    }
  }
  safeDisplay();
}

bool simGameOfLife(uint8_t *life) {
  bool change = false;
  uint8_t nextgen[(ANX_LIFE_WIDTH * ANX_LIFE_HEIGHT) / 8];
  uint8_t neighbors;

  for (int x = 0; x < ANX_LIFE_WIDTH; x++) {
    for (int y = 0; y < ANX_LIFE_HEIGHT; y++) {
      //Count the neighbors
      neighbors = 0;

      //Look Left
      if (x > 0)                                              if (gameOfLifeGet(life, x - 1, y)) neighbors++;
      //Look Up Left
      if (x > 0 && y > 0)                                     if (gameOfLifeGet(life, x - 1, y - 1)) neighbors++;
      //Look Up
      if (y > 0)                                              if (gameOfLifeGet(life, x, y - 1)) neighbors++;
      //Look up Right
      if (y > 0 && x < ANX_LIFE_WIDTH - 1)                    if (gameOfLifeGet(life, x + 1, y - 1)) neighbors++;
      //Look Right
      if (x < ANX_LIFE_WIDTH - 1)                             if (gameOfLifeGet(life, x + 1, y)) neighbors++;
      //Look Right Down
      if (x < ANX_LIFE_WIDTH - 1 && y < ANX_LIFE_HEIGHT - 1)  if (gameOfLifeGet(life, x + 1, y + 1)) neighbors++;
      //Look Down
      if (y < ANX_LIFE_HEIGHT - 1)                            if (gameOfLifeGet(life, x, y + 1)) neighbors++;
      //Look Left Down
      if (x > 0 && y < ANX_LIFE_HEIGHT - 1)                   if (gameOfLifeGet(life, x - 1, y + 1)) neighbors++;


      if (gameOfLifeGet(life, x, y)) {
        if (neighbors < 2 || neighbors >= 4) {
          gameOfLifeSet(nextgen, x, y, false);
          change = true;
        } else {
          gameOfLifeSet(nextgen, x, y, true);
        }
      } else {
        if (neighbors == 3) {
          change = true;
        }
        gameOfLifeSet(nextgen, x, y, neighbors == 3);
      }
    }
  }

  //Copy results back into current sim
  for (int i = 0; i < (ANX_LIFE_WIDTH * ANX_LIFE_HEIGHT) / 8; i++) {
    life[i] = nextgen[i];
  }

  return change;
}

void gameOfLife() {
  uint8_t life[(ANX_LIFE_WIDTH * ANX_LIFE_HEIGHT) / 8];

  for (int y = 0; y < ANX_LIFE_HEIGHT; y++) {
    for (int x = 0; x < ANX_LIFE_WIDTH; x++) {
      uint8_t r = random(ANX_LIFE_START_PROB);
      if (r == 0) {
        gameOfLifeSet(life, x, y, true);
      } else {
        gameOfLifeSet(life, x, y, false);
      }
    }
  }
  drawGameOfLife(life);
  deepSleep(1000);

  while (1) {
    simGameOfLife(life);
    drawGameOfLife(life);

    //Invode RF handler to deal with any packets received
    for (int i = 0; i < 5; i++) {
      //Handle tasks
      tick();
      deepSleep(50);
    }

    uint8_t button = getButtonState();

    if (button > 0) {
      clearButtonState();
    }

    //Annihilation if enter pressed
    if (button == BUTTON_ENTER) {
      uint8_t x = random(ANX_LIFE_WIDTH - ANX_LIFE_ANNIHILATION_W - 1);
      uint8_t y = random(ANX_LIFE_HEIGHT - ANX_LIFE_ANNIHILATION_H - 1);
      for (uint8_t yi = 0; yi < ANX_LIFE_ANNIHILATION_H; yi++) {
        for (uint8_t xi = 0; xi < ANX_LIFE_ANNIHILATION_W; xi++) {
          gameOfLifeSet(life, x + xi, y + yi, false);
        }
      }

      for (uint8_t i = 0; i < 4; i++) {
        display.fillRect(x, y, ANX_LIFE_ANNIHILATION_W, ANX_LIFE_ANNIHILATION_H, WHITE);
        safeDisplay();
        deepSleep(100);

        display.fillRect(x, y, ANX_LIFE_ANNIHILATION_W, ANX_LIFE_ANNIHILATION_H, BLACK);
        safeDisplay();
        deepSleep(100);
      }
    }

    //Add a glider randomly if down pressed
    if (button == BUTTON_DOWN) {
      uint8_t x = random(ANX_LIFE_WIDTH - 3);
      uint8_t y = random(ANX_LIFE_HEIGHT - 3);
      for (uint8_t yi = 0; yi < 3; yi++) {
        for (uint8_t xi = 0; xi < 3; xi++) {
          gameOfLifeSet(life, x + xi, y + yi, (glider[yi][xi] == 1));
        }
      }
    }

    //Add a exploder randomly if up pressed
    if (button == BUTTON_UP) {
      uint8_t x = random(ANX_LIFE_WIDTH - 3);
      uint8_t y = random(ANX_LIFE_HEIGHT - 4);
      for (uint8_t yi = 0; yi < 4; yi++) {
        for (uint8_t xi = 0; xi < 3; xi++) {
          gameOfLifeSet(life, x + xi, y + yi, (exploder[yi][xi] == 1));
        }
      }
    }


    //Add a spaceship randomly if right pressed
    if (button == BUTTON_RIGHT) {
      uint8_t x = random(ANX_LIFE_WIDTH - 5);
      uint8_t y = random(ANX_LIFE_HEIGHT - 4);
      for (uint8_t yi = 0; yi < 4; yi++) {
        for (uint8_t xi = 0; xi < 5; xi++) {
          gameOfLifeSet(life, x + xi, y + yi, (spaceship[yi][xi] == 1));
        }
      }
    }

    //Quit when they press left button
    if (button == BUTTON_LEFT) {
      break;
    }
  }
}
