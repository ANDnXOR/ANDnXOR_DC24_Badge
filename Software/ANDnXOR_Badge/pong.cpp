#include <Adafruit_NeoPixel-ANDnXOR.h>

#include "ANX.h"
#include "buttons.h"
#include "pong.h"

extern Adafruit_NeoPixel  leds;

/**
   Handle the second player as AI
*/
static void _ai(pongState *state) {
  //Only move AI occasionally
  if (rtMillis() - state->lastAIMove < AI_MIN_DELAY) {
    return;
  }

  int16_t center = state->player2Y + (PADDLE_HEIGHT / 2);
  if (state->ballY > center) {
    state->player2Y++;
  } else if (state->ballY < center) {
    state->player2Y--;
  }

  if (state->player2Y < 0)
    state->player2Y = 0;
  if (state->player2Y + PADDLE_HEIGHT > display.height())
    state->player2Y = state->player2Y - PADDLE_HEIGHT;

  state->lastAIMove = rtMillis();
}

/**
   Draw the screen based on current game state
*/
static void _drawScreen(pongState state) {
  uint8_t player1X = 0;
  uint8_t player2X = display.width() - PADDLE_WIDTH;

  display.clearDisplay();
  display.drawFastHLine(0, 0, display.width(), WHITE);
  display.drawFastHLine(0, display.height() - 1, display.width(), WHITE);
  display.fillRoundRect(player1X, state.player1Y, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_RADIUS, WHITE);
  display.fillRoundRect(player2X, state.player2Y, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_RADIUS, WHITE);
  display.fillCircle(state.ballX, state.ballY, BALL_RADIUS, WHITE);

  safeDisplay();
}

/**
   Handle when the player loses
*/
static void _lost(pongState state) {
  //Give them XP
  addExperience(state.xp);

  //All LEDS red
  setAllLeds(255, 0, 0);

  for (uint8_t y = 0; y < display.height(); y += 2) {
    display.drawFastHLine(0, y, display.width(), WHITE);
    deepSleep(40);
    safeDisplay();
  }

  clearButtonState();
  char buffer[30];
  sprintf(buffer, "You Lost\n%d XP", state.xp);
  statusDialog(buffer);

  //Wait for the user
  safeWaitForButton();

  //Clear the screen
  for (uint8_t y = 0; y < display.height(); y++) {
    display.drawFastHLine(0, y, display.width(), BLACK);
    deepSleep(10);
    safeDisplay();
  }
  return;
}

/**
   Handle when the player wins
*/
static void _won(pongState state) {
  //Give them XP
  state.xp += PONG_WIN_XP;
  addExperience(state.xp);

  //All LEDs to Green
  setAllLeds(0, 255, 0);

  for (uint8_t y = 0; y < display.height(); y += 2) {
    display.drawFastHLine(0, y, display.width(), WHITE);
    deepSleep(40);
    safeDisplay();
  }

  clearButtonState();
  char buffer[30];
  sprintf(buffer, "Victory\n%d XP", state.xp);
  statusDialog(buffer);

  safeWaitForButton();

  for (uint8_t y = 0; y < display.height(); y++) {
    display.drawFastHLine(0, y, display.width(), BLACK);
    deepSleep(10);
    safeDisplay();
  }
  return;
}

void doPong() {
  pongState state;

  state.xp = 0;
  uint8_t left[] = {0, 1, 2, 7};
  uint8_t right[] = {3, 4, 5, 6};
  state.lastAIMove = 0;

  state.ballX = display.width() / 2;
  state.ballY = display.height() / 2;
  state.ballVX = 0 - (random(BALL_VELOCITY - 2) + 2);
  state.ballVY = BALL_VELOCITY - state.ballVX;
  const uint8_t minBallX = PADDLE_WIDTH;
  const uint8_t maxBallX = display.width() - BALL_RADIUS - PADDLE_WIDTH;
  const uint8_t minBallY = BALL_RADIUS + 1;
  const uint8_t maxBallY = display.height() - BALL_RADIUS - 1;

  state.player1Y  = (display.height() - PADDLE_HEIGHT) / 2;
  state.player2Y = state.player1Y;
  const uint8_t maxPlayerY = display.height() - PADDLE_HEIGHT - 1;

  //Initialize screen
  _drawScreen(state);

  //Do a countdown for the player
  char buff[10];
  statusDialog("Get Ready!");
  deepSleep(700);
  for (uint8_t i = 3; i > 0; i--) {
    sprintf(buff, "%d", i);
    statusDialog(buff);
    deepSleep(700);
  }
  statusDialog("Go!");
  deepSleep(700);

  //main loop for the game
  while (1) {
    uint8_t button = getButtonState();
    if ((button & BUTTON_DOWN) > 0) {
      state.player1Y += PADDLE_SPEED;
    } else if ((button & BUTTON_UP) > 0) {
      state.player1Y -= PADDLE_SPEED;
    }

    if (state.player1Y > maxPlayerY) state.player1Y = maxPlayerY;
    if (state.player1Y < 1) state.player1Y = 1;
    _drawScreen(state);

    deepSleep(PONG_STEP);

    //Move the ball
    state.ballX += state.ballVX;
    state.ballY += state.ballVY;

    //Handle Artificial "intelligence"
    _ai(&state);

    //Determine if collision with player 1 paddle
    if (state.ballX <= minBallX) {

      //If ball collides, calculate a new Y velocity for the ball based on where it struck
      if ((state.ballY >= state.player1Y) &&
          (state.ballY <= state.player1Y + PADDLE_HEIGHT)) {
        state.ballX = minBallX;
        state.ballVY = BALL_VY_BIAS * (float)(state.ballY - state.player1Y - (PADDLE_HEIGHT / 2)) / (float)(PADDLE_HEIGHT / 2);

        //Make sure VY is never 0
        while (state.ballVY == 0) {
          state.ballVY = ((float)random(20) / 10.0) - 1.0; //VY is -1 to 1
        }
        state.ballVX = sqrt(pow(BALL_VELOCITY, 2) - pow(state.ballVY, 2));

        //Set all LEDs to blue
        for (uint8_t i = 0; i < 4; i++) {
          leds.setPixelColor(left[i], 0, 0, 255);
        }
        leds.show();

        //Wait
        deepSleep(100);

        //Turn off
        ledsOff();

        //Give them XP for deflection
        state.xp += PONG_DEFLECT_XP;
      }
    }

    //Determine if collision with AI paddle
    if (state.ballX >= maxBallX) {

      //If ball collides, calculate a new Y velocity for the ball based on where it struck
      if ((state.ballY >= state.player2Y) &&
          (state.ballY <= state.player2Y + PADDLE_HEIGHT)) {
        state.ballX = maxBallX;
        state.ballVY = BALL_VY_BIAS * (float)(state.ballY - state.player2Y - (PADDLE_HEIGHT / 2)) / (float)(PADDLE_HEIGHT / 2);

        //Make sure VY is never 0
        while (state.ballVY == 0) {
          state.ballVY = ((float)random(20) / 10.0) - 1.0; //VY is -1 to 1
        }

        state.ballVX = 0 - sqrt(pow(BALL_VELOCITY, 2) - pow(state.ballVY, 2));

        for (uint8_t i = 0; i < 4; i++) {
          leds.setPixelColor(right[i], 0, 0, 255);
        }
        leds.show();
        deepSleep(100);
        for (uint8_t i = 0; i < 4; i++) {
          leds.setPixelColor(right[i], 0, 0, 0);
        }
        leds.show();
      }
    }

    //Determine if the player lost
    if (state.ballX + BALL_RADIUS <= 0) {
      _lost(state);
      return;
    }

    //Determine if the player won
    if (state.ballX - BALL_RADIUS >= display.width()) {
      _won(state);
      return;
    }

    //Determine if they hit the top
    if (state.ballY <= minBallY) {
      state.ballVY = 0 - state.ballVY;
      state.ballY = minBallY;
    }

    //Determine if they hit the bottom
    if (state.ballY >= maxBallY) {
      state.ballVY = 0 - state.ballVY;
      state.ballY = maxBallY;
    }
  }
}
