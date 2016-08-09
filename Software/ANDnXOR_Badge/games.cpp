#include "ANX.h"
#include "buttons.h"
#include "games.h"
#include "graphics.h"
#include "rf.h"
#include "settings.h"

/**
   Draw asteroids game given a specific state
*/
static void _asteroidsDraw(int16_t x, int16_t y, int16_t rotate, skull *skulls, bullet *bullets, uint8_t level, bitmap skull) {
  display.clearDisplay();

  uint32_t ship = 0;
  if (rotate == 0) ship = ASTEROIDS_SHIP0_address;
  if (rotate == 45) ship = ASTEROIDS_SHIP45_address;
  if (rotate == 90) ship = ASTEROIDS_SHIP90_address;
  if (rotate == 135) ship = ASTEROIDS_SHIP135_address;
  if (rotate == 180) ship = ASTEROIDS_SHIP180_address;
  if (rotate == 225) ship = ASTEROIDS_SHIP225_address;
  if (rotate == 270) ship = ASTEROIDS_SHIP270_address;
  if (rotate == 315) ship = ASTEROIDS_SHIP315_address;

  //Draw the skulls
  for (uint8_t i = 0; i < ASTEROIDS_NUMBER_SKULLS; i++) {
    if (skulls[i].x >= 0 && skulls[i].y >= 0) {
      drawBitmapFlash(skull, skulls[i].x, skulls[i].y);
    }
  }

  //Draw the bullets
  for (uint8_t i = 0; i < ASTEROIDS_NUMBER_BULLETS; i++) {
    if (bullets[i].x >= 0 && bullets[i].y >= 0) {
      display.drawPixel(bullets[i].x, bullets[i].y, WHITE);
    }
  }

  //Draw the ship
  bitmap bmp = getBitmapMetadata(ship);
  drawBitmapFlash(bmp, x, y);

  //Level
  display.setCursor(0, 0);
  display.print("Level ");
  display.print(level);

  safeDisplay();
}

/**
   Draw the dodge world based for a given state
*/
static void _dodgeDraw(dodgeState state) {
  display.clearDisplay();

  //Draw the falling objects
  for (uint8_t i = 0; i < DODGE_SPRITE_COUNT; i++) {
    if (state.sprites[i].x >= 0) {
      bitmap bmp = getBitmapMetadata(state.sprites[i].address);
      drawBitmapFlash(bmp, state.sprites[i].x, state.sprites[i].y);
    }
  }

  //Draw the player
  bitmap bmp = getBitmapMetadata(FLAPPY_SKULL_DOWN_address);
  drawBitmapFlash(bmp, state.x, display.height() - bmp.height);

  //Print the points
  display.setCursor(0, 0);
  display.print("Points: ");
  display.print(state.points);

  safeDisplay();
}

/**
   Draw flappy bird based on current state
*/
static void _flappyDraw(flappyState state, bitmap skullDown, bitmap skullUp, bitmap pipeBottom, bitmap pipeTop) {
  display.clearDisplay();

  //Draw the correct player images
  if (state.flapUp)
    display.drawBitmap(FLAPPY_PLAYER_X, state.playerY, state.skullUp, skullUp.width, skullUp.height, WHITE);
  else
    display.drawBitmap(FLAPPY_PLAYER_X, state.playerY, state.skullDown, skullDown.width, skullDown.height, WHITE);

  //Draw the pipes
  for (uint8_t i = 0; i < FLAPPY_PIPE_COUT; i++) {
    if (state.pipes[i].x <= display.width()) {
      int8_t topY = (0 - pipeTop.height) + state.pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT;
      int8_t bottomY = state.pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT + FLAPPY_PIPE_GAP;
      display.drawBitmap(state.pipes[i].x, topY, state.pipeTop, pipeTop.width, pipeTop.height, WHITE);
      display.drawBitmap(state.pipes[i].x, bottomY, state.pipeBottom, pipeBottom.width, pipeBottom.height, WHITE);
    }
  }

  //Draw XP
  display.setCursor(0, 0);
  display.print(state.pipesPassed);

  //Draw ground
  display.drawFastHLine(0, display.height() - 1, display.width(), WHITE);

  safeDisplay();
}

/**
   Initialize the skulls
*/
void _initAsteroidsSkulls(skull *skulls) {
  bitmap skull = getBitmapMetadata(FLAPPY_SKULL_DOWN_address);

  //Randomly initialize the skulls
  for (uint8_t i = 0; i < ASTEROIDS_NUMBER_SKULLS; i++) {

    //Randomly pick a border to init on
    switch (random(4)) {
      case 0:
        skulls[i].x = random(display.width());
        skulls[i].y = 0;
        break;
      case 1:
        skulls[i].x = 0;
        skulls[i].y = random(display.height());
        break;
      case 2:
        skulls[i].x = random(display.width());
        skulls[i].y = display.height() - skull.height;
        break;
      case 3:
        skulls[i].x = display.width() - skull.width;
        skulls[i].y = random(display.height());
        break;
    }

    //Randomly select a vector
    do {
      skulls[i].dx = (random(ASTEROIDS_MAX_DX * 20.0) - ASTEROIDS_MAX_DX) / 10.0;
      skulls[i].dy = (random(ASTEROIDS_MAX_DY * 20.0) - ASTEROIDS_MAX_DY) / 10.0;
    } while (skulls[i].dx == 0 && skulls[i].dy == 0);
  }
}

/**
   Handle game over event
*/
static void _lost(uint32_t xp) {
  setAllLeds(255, 0, 0);

  //Give them credit
  addExperience(xp);

  char buffer[32];
  sprintf(buffer, "Game Over\n%d XP", xp);
  statusDialog(buffer);
  safeDisplay();
  safeWaitForButton();

  ledsOff();
}

/**
   Draw the ski game based on a given state
*/
static void _skiDraw(skiState state) {
  display.clearDisplay();

  //Draw the sprites
  for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
    sprite s = state.sprites[i];
    drawBitmapFlash(getBitmapMetadata(s.address), s.x, s.y);
  }

  //Draw the skier
  switch (state.angle) {
    case -45:
      drawBitmapFlash(getBitmapMetadata(SKI_LEFT_address), SKI_X, SKI_Y);
      break;
    case 0:
      drawBitmapFlash(getBitmapMetadata(SKI_address), SKI_X, SKI_Y);
      break;
    case 45:
      drawBitmapFlash(getBitmapMetadata(SKI_RIGHT_address), SKI_X, SKI_Y);
      break;
  }

  display.setCursor(0, 0);
  display.print(state.distance);
  display.print("m");

  safeDisplay();
}

/**
   Primary asteroids game loop
*/
void asteroids() {
  disablePopups();

  bitmap shipBmp = getBitmapMetadata(ASTEROIDS_SHIP0_address);
  bitmap skullBmp = getBitmapMetadata(FLAPPY_SKULL_DOWN_address);

  //Ship's position and velocity
  float x = (display.width() / 2) - (shipBmp.width / 2);
  float y = (display.height() / 2) - (shipBmp.height / 2);
  float dx = 0;
  float dy = 0;

  //Game timing
  uint32_t skullsDestroyed = 0;
  uint32_t lastBulletTime = 0;
  uint32_t lastGameStep = 0;
  bullet bullets[ASTEROIDS_NUMBER_BULLETS];
  uint8_t bulletIndex = 0;
  skull skulls[ASTEROIDS_NUMBER_SKULLS];
  int16_t rotate = 0;
  uint8_t level = 1;
  uint8_t skullsLeft = ASTEROIDS_NUMBER_SKULLS;

  //Initialize the bullets
  for (uint8_t i = 0; i < ASTEROIDS_NUMBER_BULLETS; i++) {
    bullets[i].x = -1;
    bullets[i].y = -1;
  }

  _initAsteroidsSkulls(skulls);

  while (1) {
    _asteroidsDraw(x, y, rotate, skulls, bullets, level, skullBmp);

    //If it's time to advance the game...
    if (rtMillis() - lastGameStep > ASTEROIDS_GAME_STEP_MS) {

      //Move skulls
      for (uint8_t i = 0; i < ASTEROIDS_NUMBER_SKULLS; i++) {
        skulls[i].x += skulls[i].dx;
        skulls[i].y += skulls[i].dy;

        //wrap around
        if (skulls[i].x < (0 - skullBmp.width))
          skulls[i].x = display.width();
        if (skulls[i].x > display.width())
          skulls[i].x = 0 - skullBmp.width;
        if (skulls[i].y < (0 - skullBmp.height))
          skulls[i].y = display.height();
        if (skulls[i].y > display.height())
          skulls[i].y = 0 - skullBmp.height;
      }

      //Move the bullets
      for (uint8_t i = 0; i < ASTEROIDS_NUMBER_BULLETS; i++) {
        //If the bullet is in bounds, move it
        if (bullets[i].x >= 0 &&
            bullets[i].x < display.width() &&
            bullets[i].y >= 0 &&
            bullets[i].y < display.height()) {
          bullets[i].x += bullets[i].dx;
          bullets[i].y += bullets[i].dy;

          //Collision detection
          for (uint8_t j = 0; j < ASTEROIDS_NUMBER_SKULLS; j++) {
            //Bullet is within bounds of a skull
            if (bullets[i].x < (skulls[j].x + skullBmp.width) && bullets[i].x > skulls[j].x &&
                bullets[i].y < (skulls[j].y + skullBmp.height) && bullets[i].y > skulls[j].y) {
              skulls[j].x = -100;
              skulls[j].y = -100;
              skulls[j].dx = 0;
              skulls[j].dy = 0;

              //Flash the LEDs
              setAllLeds(50, 50, 200);
              deepSleep(200);
              ledsOff();

              skullsLeft--;
              skullsDestroyed++;

              //They won this level
              if (skullsLeft == 0) {
                level++;
                statusDialog("Level Completed");
                safeDisplay();
                deepSleep(500);
                clearButtonState();
                waitForButton();
                clearButtonState();
                _initAsteroidsSkulls(skulls);
                skullsLeft = ASTEROIDS_NUMBER_SKULLS;
              }
            }
          }
        }
      }

      //Move the ship
      x += dx;
      y += dy;

      //Wrap the ship around
      if (x < 0 - shipBmp.width) x = display.width();
      if (x > display.width()) x = 0 - shipBmp.width;
      if (y < 0 - shipBmp.height) y = display.height();
      if (y > display.height()) y = 0 - shipBmp.height;

      //Detect ship collision with asteroids
      for (uint8_t i = 0; i < ASTEROIDS_NUMBER_SKULLS; i++) {

        //Does the ship collide with the skull?
        if (x + shipBmp.width - ASTEROIDS_SHIP_MARGIN > skulls[i].x &&
            x + ASTEROIDS_SHIP_MARGIN < skulls[i].x + skullBmp.width &&
            y + shipBmp.height - ASTEROIDS_SHIP_MARGIN > skulls[i].y &&
            y + ASTEROIDS_SHIP_MARGIN < skulls[i].y + skullBmp.height) {
          _lost(skullsDestroyed * ASTEROIDS_XP_PER_SKULL);
          enablePopups();
          return;
        }
      }

      lastGameStep = rtMillis();
    }


    uint8_t button = getButtonState();

    safeDisplay();

    //Rotate left
    if ((button & BUTTON_LEFT) > 0) {
      rotate -= 45;
      if (rotate < 0) rotate += 360;
    }

    //Rotate right
    if ((button & BUTTON_RIGHT) > 0) {
      rotate += 45;
      if (rotate >= 360) rotate -= 360;
    }

    //Accelerate
    if ((button & BUTTON_UP) > 0) {
      switch (rotate) {
        case 0:
          dy -= ASTEROIDS_SHIP_ACCEL;
          break;
        case 45:
          dx += sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          dy -= sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          break;
        case 90:
          dx += ASTEROIDS_SHIP_ACCEL;
          break;
        case 135:
          dx += sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          dy += sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          break;
        case 180:
          dy += ASTEROIDS_SHIP_ACCEL;
          break;
        case 225:
          dx -= sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          dy += sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          break;
        case 270:
          dx -= ASTEROIDS_SHIP_ACCEL;
          break;
        case 315:
          dx -= sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          dy -= sqrt(2 * ASTEROIDS_SHIP_ACCEL);
          break;
      }

      if (dx < 0 - ASTEROIDS_MAX_DX) dx = 0 - ASTEROIDS_MAX_DX;
      if (dx > ASTEROIDS_MAX_DX) dx = ASTEROIDS_MAX_DX;
      if (dy < 0 - ASTEROIDS_MAX_DY) dy = 0 - ASTEROIDS_MAX_DY;
      if (dy > ASTEROIDS_MAX_DY) dy = ASTEROIDS_MAX_DY;
    }

    //Fire gun
    if ((button & BUTTON_ENTER) > 0) {
      //Fire a bullet
      if (rtMillis() - lastBulletTime > ASTEROIDS_BULLET_STEP_MS) {

        switch (rotate) {
          case 0:
            bullets[bulletIndex].x = x + 9;
            bullets[bulletIndex].y = y;
            bullets[bulletIndex].dx = 0;
            bullets[bulletIndex].dy = -3;
            break;
          case 45:
            bullets[bulletIndex].x = x + 17;
            bullets[bulletIndex].y = y + 2;
            bullets[bulletIndex].dx = 2.44;
            bullets[bulletIndex].dy = -2.44;
            break;
          case 90:
            bullets[bulletIndex].x = x + 19;
            bullets[bulletIndex].y = y + 9;
            bullets[bulletIndex].dx = 3;
            bullets[bulletIndex].dy = 0;
            break;
          case 135:
            bullets[bulletIndex].x = x + 17;
            bullets[bulletIndex].y = y + 17;
            bullets[bulletIndex].dx = 2.44;
            bullets[bulletIndex].dy = 2.44;
            break;
          case 180:
            bullets[bulletIndex].x = x + 10;
            bullets[bulletIndex].y = y + 19;
            bullets[bulletIndex].dx = 0;
            bullets[bulletIndex].dy = 3;
            break;
          case 225:
            bullets[bulletIndex].x = x + 2;
            bullets[bulletIndex].y = y + 17;
            bullets[bulletIndex].dx = -2.44;
            bullets[bulletIndex].dy = 2.44;
            break;
          case 270:
            bullets[bulletIndex].x = x;
            bullets[bulletIndex].y = y + 9;
            bullets[bulletIndex].dx = -3;
            bullets[bulletIndex].dy = 0;
            break;
          case 315:
            bullets[bulletIndex].x = x + 2;
            bullets[bulletIndex].y = y + 2;
            bullets[bulletIndex].dx = -2.44;
            bullets[bulletIndex].dy = -2.44;
            break;
        }

        bulletIndex = (bulletIndex + 1) % ASTEROIDS_NUMBER_BULLETS;
        lastBulletTime = rtMillis();
      }
    }

    deepSleep(100);
    tick();
  }
}

/**
   Primary function for running dodge game
*/
void dodge() {
  disablePopups();
  bitmap skull = getBitmapMetadata(FLAPPY_SKULL_DOWN_address);
  bitmap virus = getBitmapMetadata(DODGE_VIRUS_address);

  uint32_t lastGameStep = 0;
  uint32_t lastPlayerStep = 0;
  uint8_t cols = display.width() / virus.width;
  dodgeState state;

  //Initialie the player
  state.x = 0;
  state.points = 0;

  //Initialize the falling objects
  for (uint8_t i = 0; i < DODGE_SPRITE_COUNT; i++) {
    state.sprites[i].x = -1;
    state.sprites[i].y = -1;
  }

  //Main game loop
  while (1) {
    _dodgeDraw(state);

    if (rtMillis() - lastGameStep > DODGE_STEP_MS) {

      //advance existing objects
      for (uint8_t i = 0; i < DODGE_SPRITE_COUNT; i++) {
        if (state.sprites[i].x >= 0) {
          state.sprites[i].y += DODGE_FALL_VELOCITY;

          //Detect if a sprite falls off the screen
          if (state.sprites[i].y >= display.height()) {
            state.sprites[i].x = -1;
            state.sprites[i].y = -1;

            //Subtract points for missed floppies
            if (state.sprites[i].address == DODGE_FLOPPY_address) {
              setAllLeds(200, 50, 50);
              state.points += DODGE_POINTS_PER_MISS;
              if (state.points < 0) state.points = 0;
              deepSleep(200);
              ledsOff();
            }
          }

          //Is this object low enough for a collision?
          if (state.sprites[i].y + virus.height > display.height() - skull.height) {

            //If the user is in teh same column, there's a collision
            if (state.x == state.sprites[i].x) {
              if (state.sprites[i].address == DODGE_FLOPPY_address) {
                setAllLeds(50, 200, 50);
                state.points += DODGE_POINTS_PER_FLOPPY;
                state.sprites[i].x = -1;
                state.sprites[i].y = -1;
                deepSleep(200);
                ledsOff();
              } else {
                _lost(state.points * DODGE_XP_PER_POINT);
                enablePopups();
                return;
              }
            }
          }
        }
      }

      //Add an object (maybe)
      if (random(100) < DODGE_CREATE_OBJECT_PCT) {
        uint8_t i = random(cols); //pick a random column

        bool objectCreated = false;
        for (uint8_t j = 0; j < DODGE_SPRITE_COUNT; j++) {
          if (state.sprites[j].x == -1 && !objectCreated) {
            state.sprites[j].x = i * virus.width;
            state.sprites[j].y = 0;
            switch (random(4)) {
              case 0:
                state.sprites[j].address = DODGE_VIRUS_address;
                break;
              case 1:
                state.sprites[j].address = DODGE_LOCK_address;
                break;
              case 2:
                state.sprites[j].address = DODGE_EDGE_address;
                break;
              case 3:
                state.sprites[j].address = DODGE_FLOPPY_address;
                break;
            }
            objectCreated = true;
          }
        }
      }


      lastGameStep = rtMillis();
    }


    //Handle buttons
    uint8_t button = getButtonState();

    //Only listen to player events every so often
    if (rtMillis() - lastPlayerStep > DODGE_PLAYER_STEP_MS) {
      if ((button & BUTTON_LEFT) > 0) {
        state.x -= skull.width;
        if (state.x < 0) state.x = 0;
      }
      if ((button & BUTTON_RIGHT) > 0) {
        state.x += skull.width;
        if (state.x >= display.width() - skull.width) state.x = display.width() - skull.width;
      }
      lastPlayerStep = rtMillis();
    }

    //zzzz
    deepSleep(50);
    tick();
  }
}



/**
   Primary entry point for flappy DEFCON
*/
void flappy() {
  disablePopups();
  flappyState state;
  state.xp = 0;
  state.pipesPassed = 0;
  state.playerY = 32;
  uint32_t flapEndTime = 0;
  uint32_t lastGameStep = 0;
  float velocity = 0;

  bitmap skullDown = getBitmapMetadata(FLAPPY_SKULL_DOWN_address);
  bitmap skullUp = getBitmapMetadata(FLAPPY_SKULL_UP_address);
  bitmap pipeBottom = getBitmapMetadata(FLAPPY_PIPE_BOTTOM_address);
  bitmap pipeTop = getBitmapMetadata(FLAPPY_PIPE_TOP_address);

  //Load flappy skull
  flash.readBytes(skullDown.address, &state.skullDown, state.skullSizeBytes);
  flash.readBytes(skullUp.address, &state.skullUp, state.skullSizeBytes);
  //Load pipes
  flash.readBytes(pipeBottom.address, &state.pipeBottom, state.pipeSizeBytes);
  flash.readBytes(pipeTop.address, &state.pipeTop, state.pipeSizeBytes);

  //initialize the pipes
  for (uint8_t i = 0; i < FLAPPY_PIPE_COUT; i++) {
    state.pipes[i].x =  display.width() + (i * FLAPPY_PIPE_SPACING);
    state.pipes[i].h = random(display.height() - (2 * FLAPPY_PIPE_MIN_HEIGHT) - FLAPPY_PIPE_GAP);
    state.pipes[i].passed = false;
  }

  //Primary game loop
  while (1) {
    state.flapUp = rtMillis() < flapEndTime;
    _flappyDraw(state, skullDown, skullUp, pipeBottom, pipeTop);

    if (rtMillis() - lastGameStep > FLAPPY_STEP_MS) {
      if (state.flapUp) {
        velocity = FLAPPY_VELOCITY_UP;
      }

      //Player falls
      state.playerY += velocity;
      //Accelerates towards ground due to gravity
      velocity += FLAPPY_GRAVITY;
      if (velocity >= FLAPPY_MAX_VELOCITY)
        velocity = FLAPPY_MAX_VELOCITY;

      //Move the pipes and detect collisions
      for (int8_t i = 0; i < FLAPPY_PIPE_COUT; i++) {
        state.pipes[i].x -= FLAPPY_PIPE_VELOCITY;

        //Detect if the pipe goes off the screen
        if (state.pipes[i].x <= 0 - pipeTop.width) {
          //Wrap the pipe after the last pipe
          state.pipes[i].x = state.pipes[(i + FLAPPY_PIPE_COUT - 1) % FLAPPY_PIPE_COUT].x + FLAPPY_PIPE_SPACING;
          //Randomly pick a new height
          state.pipes[i].h = random(display.height() - (2 * FLAPPY_PIPE_MIN_HEIGHT) - FLAPPY_PIPE_GAP);
          //reset passed value
          state.pipes[i].passed = false;
        }

        //Detect if this pipe is passing by the player
        if ((state.pipes[i].x + pipeTop.width >= FLAPPY_PLAYER_X) &&
            (state.pipes[i].x <= FLAPPY_PLAYER_X + skullDown.width)) {

          //Detect if player is hitting a pipe
          if ((state.playerY < state.pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT) ||
              (state.playerY + skullDown.height > state.pipes[i].h + FLAPPY_PIPE_MIN_HEIGHT + FLAPPY_PIPE_GAP)) {
            _lost(state.xp);
            enablePopups();
            return;
          }
        }

        //Count some XP for the user if it's behind them
        if ((state.pipes[i].x + pipeTop.width) < FLAPPY_PLAYER_X) {
          if (!state.pipes[i].passed) {
            state.pipes[i].passed = true;
            state.pipesPassed++;
            state.xp += state.pipesPassed;
          }
        }
      }

      //Detect ground collision
      if (state.playerY + skullDown.height >= display.height()) {
        _lost(state.xp);
        enablePopups();
        return;
      }

      lastGameStep = rtMillis();
    }

    uint8_t button = getButtonState();
    //Detect up button, flap up
    if (button & BUTTON_UP > 0) {
      flapEndTime = rtMillis() + FLAPPY_UP_TIME_MS;
    }
    //Detect if they want to quit
    else if (button & BUTTON_LEFT > 0) {
      clearButtonState();
      enablePopups();
      return;
    }

    //Delay and do work
    deepSleep(50);
    tick();
  }
}

/**
   Display game progress to user
*/
void gameProgress() {
  window("Game Progress");
  display.setCursor(1, 20);
  display.print("XP: ");
  display.print(ANXGetExperience());
  display.print("/");
  display.println(XP_PER_LEVEL);
  display.print("Level: ");
  display.println(ANXGetLevel());
  safeDisplay();
  safeWaitForButton();
}

/**
   Primary ski game function
*/
void ski() {
  uint32_t lastGameStep = 0;
  uint32_t addresses[] = {SKI_FLAG_address, SKI_ROCK_address, SKI_TREE_address};

  bitmap ski = getBitmapMetadata(SKI_address);

  skiState state;
  state.angle = 0;
  state.distance = 0;
  state.velocity = 2.0;
  state.velocityAngled = sqrt(pow(state.velocity, 2));

  //Initialize the sprites
  for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
    state.sprites[i].x = random(display.width() * SKI_WORLD_WIDTH) - display.width();   //Generate sprites three screens wide
    state.sprites[i].y = random(display.height() * SKI_WORLD_HEIGHT) + display.height();
    state.sprites[i].address = addresses[random(3)];
  }

  while (1) {
    _skiDraw(state);

    if (rtMillis() - lastGameStep > SKI_STEP_MS) {
      //Move the sprites (not the skier)
      float dx, dy;

      //Determine the velocities
      switch (state.angle) {
        case -45:
          dx = state.velocityAngled;
          dy = 0 - state.velocityAngled;
          break;
        case 0:
          dx = 0;
          dy = 0 - state.velocity;
          break;
        case 45:
          dx = 0 - state.velocityAngled;
          dy = 0 - state.velocityAngled;
          break;
      }

      //Move the sprites
      for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
        state.sprites[i].x += dx;
        state.sprites[i].y += dy;

        //sprite is off the screen re-generate
        if (state.sprites[i].y < 0 - ski.height) {
          state.sprites[i].x = random(display.width() * SKI_WORLD_WIDTH) - display.width();   //Generate sprites three screens wide
          state.sprites[i].y = random(display.height() * SKI_WORLD_HEIGHT) + display.height();
          state.sprites[i].address = addresses[random(3)];
        }

        //Detect collisions
        if (state.sprites[i].x + ski.width > SKI_X + SKI_MARGIN &&
            state.sprites[i].x < SKI_X + ski.width - SKI_MARGIN &&
            state.sprites[i].y + ski.height > SKI_Y + SKI_MARGIN &&
            state.sprites[i].y < SKI_Y + ski.height - SKI_MARGIN) {
          _lost(state.distance / SKI_M_PER_XP);
          enablePopups();
          return;
        }
      }

      state.distance += state.velocity;
      state.velocity += SKI_ACCELERATION;
      state.velocityAngled = sqrt(pow(state.velocity, 2));

      lastGameStep = rtMillis();
    }

    uint8_t button = getButtonState();
    if ((button & BUTTON_LEFT) > 0) {
      state.angle = -45;
    }
    if ((button & BUTTON_RIGHT) > 0) {
      state.angle = 45;
    }
    if ((button & BUTTON_DOWN) > 0) {
      state.angle = 0;
    }
    clearButtonState();
  }
}

