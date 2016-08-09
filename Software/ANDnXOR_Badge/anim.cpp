#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include "ANX.h"
#include "anim.h";
#include "buttons.h"
#include "flash.h";
#include "graphics.h"
#include "input.h"
#include "rf.h"

extern Adafruit_SSD1306   display;
extern Adafruit_NeoPixel  leds;
extern ANXFlash           flash;

/**
   Cyber pathogen animation
*/
void cyberPathogen() {
  char text[16];
  sprintf(text, "CYBER PATHOGEN");
  scroll(text, PATHOGEN_address, false, true);
}

/**
   Three rules of defcon animation
*/
void defcon() {
  while (1) {
    if (scroll("DEFCON RULES", DEFCON_address, false, false) == -1) return;
    if (scroll("3 HRS OF SLEEP", DEFCON_address, false, false) == -1) return;
    if (scroll("2 MEALS A DAY", DEFCON_address, false, false) == -1) return;
    if (scroll("1 SHOWER", DEFCON_address, false, false) == -1) return;
  }
}

/**
   EFF supporter animation
*/
void eff() {
  setEyeLeftLed(255, 0, 0);
  setEyeRightLed(255, 0, 0);
  scroll("SUPPORTER", EFF_address, false, true);
}

/**
   Call back for each flame frame
*/
void flamesCallback(uint8_t frame, uint8_t *data) {
  //Only update LEDs every few frames
  if (frame % 2 == 0) {
    //red
    leds.setPixelColor(2, randBrightness(roygbiv[0]));
    leds.setPixelColor(3, randBrightness(roygbiv[0]));
    leds.setPixelColor(1, randBrightness(roygbiv[0]));
    leds.setPixelColor(4, randBrightness(roygbiv[0]));
    //orange
    leds.setPixelColor(0, randBrightness(roygbiv[1]));
    leds.setPixelColor(5, randBrightness(roygbiv[1]));
    //yellow
    leds.setPixelColor(6, randBrightness(roygbiv[2]));
    leds.setPixelColor(7, randBrightness(roygbiv[2]));
    leds.show();
  }
}

/**
   Flames animation
*/
void flames() {
  playAnimation(FLAMES_address, 80, &flamesCallback);
}


/**
   Classic flying toasters screensaver
*/
void flyingToasters() {
  struct Toaster {
    float x;
    float y;
    float dx;
    float dy;
    uint8_t type;
  };

  Toaster icons[TOASTER_COUNT];
  randomSeed(666);     // whatever seed

  display.fillScreen(BLACK);

  // initialize
  for (uint8_t f = 0; f < TOASTER_COUNT; f++) {
    icons[f].x = display.width();
    icons[f].y = random(2 * display.height()) - display.height();
    icons[f].dy = (random(7) / 10.0) + 1.0;
    icons[f].dx = (random(13) / 10.0) + 1;
    icons[f].type = random(2);
  }

  //Load graphics into memory
  bitmap toastData = getBitmapMetadata(TOAST_address);
  bitmap toasterData = getBitmapMetadata(TOASTER_address);
  uint32_t size = ceil(toastData.width * toastData.height / 8);
  uint8_t toast[size];
  uint8_t toaster[size];
  flash.readBytes(toastData.address, &toast, size);
  flash.readBytes(toasterData.address, &toaster, size);

  //Main loop
  while (1) {

    display.clearDisplay();

    // draw each icon
    for (uint8_t f = 0; f < TOASTER_COUNT; f++) {
      if (icons[f].type == 0) {
        display.drawBitmap(icons[f].x, icons[f].y, toaster, toasterData.width, toasterData.height, WHITE);
      } else {
        display.drawBitmap(icons[f].x, icons[f].y, toast, toastData.width, toastData.height, WHITE);
      }
    }
    safeDisplay();

    deepSleep(130);

    // then erase it + move it
    for (uint8_t f = 0; f < TOASTER_COUNT; f++) {
      // move it
      icons[f].y += icons[f].dy;
      icons[f].x -= icons[f].dx;
      // if its gone, reinit
      if (icons[f].y > display.height()) {
        icons[f].x = random(display.width());
        icons[f].y = 0;
        icons[f].dy = random(6) + 1;
        icons[f].type = random(2);
      }
      if (icons[f].x <= 0 - toastData.width) {
        icons[f].x = display.width();
        icons[f].y = random(display.height());
        icons[f].dy = random(3) + 1;
        icons[f].dx = random(6) + 1;
        icons[f].type = random(2);
      }
    }

    //Handle tasks
    tick();

    //if a button was pressed (async), stop animating
    if (getButtonState() > 0) {
      safeClearButtonState();
      ledsOff();
      break;
    }
  }
}

/**
   Per frame callback for glow
*/
void glowCallback(uint8_t frame, uint8_t *data) {

}

/**
   Pulsate the LEDs. Enter button switches colors.
*/
void glow() {
  display.clearDisplay();
  safeDisplay();

  int8_t delta = 5;
  int brightness = 0;
  uint8_t mode = 0; //0 white, 1 red, 2 green, 3 blue, 4 yellow
  while (1) {

    switch (mode) {
      case 0:
        setAllLeds(brightness, brightness, brightness);
        break;
      case 1:
        setAllLeds(brightness, 0, 0);
        break;
      case 2:
        setAllLeds(0, brightness, 0);
        break;
      case 3:
        setAllLeds(0, 0, brightness);
        break;
      case 4:
        setAllLeds(brightness, brightness, 0);
        break;
    }
    deepSleep(50);

    brightness += delta;

    //Reverse
    if (brightness > 255) {
      brightness = 255;
      delta = -10;
    }
    if (brightness < 0) {
      brightness = 0;
      delta = 10;
    }

    //Handle tasks
    tick();

    uint8_t button = getButtonState();

    if (button > 0) {
      safeClearButtonState();
      if (button == BUTTON_LEFT) {
        ledsOff();
        break;
      }

      if (button == BUTTON_ENTER) {
        mode = (mode + 1) % 5;
      }
    }
  }
}

/**
   Hackaday scroll
*/
void hackaday() {
  setEyeLeftLed(255, 255, 255);
  setEyeRightLed(255, 255, 255);
  scroll("HACKADAY", HACKADAY_address, false, true);
}

static void hackersCallback(uint8_t frame, uint8_t *data) {
  //unpack the data
  float hue = (float) * data / 100.0;

  //Set the eyes
  uint32_t color = HSVtoRGB(hue, 1, 1);
  setEyeLeftLed(color);
  setEyeRightLed(color);

  //Increment the hue
  hue += .01;
  if (hue >= 1) hue = 0;

  //pack the hue back into the data
  *data = (int)(hue * 100);
}

/**
   Play hackers animation
*/
void hackers() {
  playAnimation(HACKERS_address, 100, hackersCallback);
}

/**
   Frame callback for knight rider animation
*/
static void knightRiderCallback(uint8_t frame, uint8_t *data) {
  int8_t first = frame % NUMBER_LEDS;

  //Fade LEds progressively as they get further away from first led
  for (float i = 4; i >= 0; i--) {
    uint8_t red = (int)(240.0 * (i / 4.0));
    int8_t index = first - (4 - i);
    if (index < 0) index += NUMBER_LEDS;
    leds.setPixelColor(index, red, 0, 0);
  }
  leds.show();
}

/**
   Knight Rider animation mode
*/
void knightRider() {
  playAnimation(KNIGHTRIDER_address, 100, &knightRiderCallback);
}

/**
   Lycos animated logo
*/
void lycos() {
  playAnimation(LYCOS_address, 1000, NULL);
}

/**
   Simple per-frame callback for major lazer, randomize the leds
*/
static void majorLazerCallback(uint8_t frame, uint8_t *data) {
  //Unpack the data
  float hue = (float)(*data) / 100.0;
  setAllLeds(HSVtoRGB(hue, 1, 1));

  //Increment row and color and loop around
  hue += .02;
  if (hue >= 1) hue = 0;

  //Pack the data and store for next time
  *data = (int)(hue * 100);
}

/**
   Do major lazer animation
*/
void majorLazer() {
  playAnimation(MAJOR_LAZER_address, 50, &majorLazerCallback);
}

/**
   Matrix animation of LEDs and the screen
*/
void matrix() {
  bitmap d = getBitmapMetadata(MATRIX_CHAR_d_address);
  bitmap e = getBitmapMetadata(MATRIX_CHAR_e_address);
  bitmap f = getBitmapMetadata(MATRIX_CHAR_f_address);
  bitmap c = getBitmapMetadata(MATRIX_CHAR_c_address);
  bitmap o = getBitmapMetadata(MATRIX_CHAR_o_address);
  bitmap n = getBitmapMetadata(MATRIX_CHAR_n_address);

  //Load the matrix graphics
  uint32_t size = ceil(d.width * d.height / 8);
  uint8_t graphics[6][size];
  flash.readBytes(d.address, &graphics[0], size);
  flash.readBytes(e.address, &graphics[1], size);
  flash.readBytes(f.address, &graphics[2], size);
  flash.readBytes(c.address, &graphics[3], size);
  flash.readBytes(o.address, &graphics[4], size);
  flash.readBytes(n.address, &graphics[5], size);

  safeClearButtonState();
  uint8_t rows = display.height() / d.height;
  uint8_t cols = display.width() / d.width;
  uint16_t drops[cols];
  int dropleds[NUMBER_LEDS];
  display.clearDisplay();

  //Set starting height randomly for each drop (column)
  for (int i = 0; i < cols; i++) {
    drops[i] = random(rows) * d.height; //random starting y coordinate starting
  }

  //Set starting brightness for each LED
  for (int i = 0; i < NUMBER_LEDS; i++) {
    dropleds[i] = random(255);
  }

  while (1) {
    //Move each drop down
    for (int i = 0; i < cols; i++) {
      int x = i * d.width;

      display.drawBitmap(x, drops[i], graphics[random(6)], d.width, d.height, WHITE);

      //erase a previous drop
      int prev = drops[i] - (3 * d.height);
      if (prev < 0) prev += display.height();
      display.fillRect(x, prev, d.width, d.height, BLACK);

      drops[i] += d.height;

      //Wrap around
      if (drops[i] > display.height()) {
        drops[i] = 0;
      }
    }
    safeDisplay();

    //Dim and set each LED drop
    for (int i = 0; i < NUMBER_LEDS; i++) {
      dropleds[i] -= random(10) + 15;
      if (dropleds[i] < -200) dropleds[i] = 255;
      if (dropleds[i] > 0) {
        leds.setPixelColor(i, leds.Color(0, dropleds[i], 0));
      } else {
        leds.setPixelColor(i, leds.Color(0, 0, 0));
      }
    }

    leds.show();

    deepSleep(50);
    //Handle tasks
    tick();

    //if a button was pressed, stop animating
    if (getButtonState() > 0) {
      safeClearButtonState();
      ledsOff();
      break;
    }
  }
}

/**
   Special per-frame callback from nayan cat, simply animate LEDs
*/
static void nayanCallback(uint8_t frame, uint8_t *data) {
  uint8_t i = random(NUMBER_LEDS);
  leds.setPixelColor(i, 255, 255, 255);
  leds.show();
  deepSleep(60);

  leds.setPixelColor(i, roygbiv[*data]);
  leds.show();

  *data = (*data + 1) % 7;
}

/**
   Nayan cat animation
*/
void nayan() {
  playAnimation(NAYAN_address, 40, &nayanCallback);
}

/**
   Special callback for each frame of the netscape animation
*/
void netscapeCallback(uint8_t frame, uint8_t *data) {
  uint16_t b = 0;

  //If certain frames are being show, determine a brightness value for them
  if (frame >= 25 && frame <= 33) {
    b = (255 * (frame - 25)) / 9;
  }


  //Set the brightness of the LEDs
  setAllLeds(b, b, b);
}

/**
   Show netscape "throbber" animation
*/
void netscape() {
  playAnimation(NETSCAPE_address, 100, &netscapeCallback);
}

/**
   Callback for each frame for party mode
*/
void partyCallback(uint8_t frame, uint8_t *data) {
  if (frame % 2 == 0) {
    for (uint8_t i = 0; i < NUMBER_LEDS; i++) {
      leds.setPixelColor(i, roygbiv[random(7)]);
    }
    leds.show();
  }
}

/**
   Dancing party animation
*/
void party(uint8_t d) {
  uint8_t animCount = 3; //purposely limit to 3 unless they unlocked
  uint8_t anim = 0;
  uint8_t button = 0;

  //expose two more party animations
  if ((ANXGetUnlocked() & UNLOCK_MASTER) > 0) {
    animCount = 5;
  }

  while (1) {
    switch (anim) {
      case 0:
        button = playAnimation(PARTY_address, d, &partyCallback);
        break;
      case 1:
        button = playAnimation(PARTY_BENDER_address,  d, &partyCallback);
        break;
      case 2:
        button = playAnimation(PARTY_TOAD_address, d, &partyCallback);
        break;
      case 3:
        button = playAnimation(PARTY_ZOIDBURG_address, d, &partyCallback);
        break;
      case 4:
        button = playAnimation(PARTY_FRY_address, d, &partyCallback);
        break;
    }

    //Advance the animation looping around
    if ((button & BUTTON_ENTER) > 0) {
      anim = (anim + 1) % animCount;
    } else if ((button & BUTTON_LEFT) > 0) {
      return;
    }
  }
}

/**
   Per-frame callback for pirate animation. Do something with the LEDs.
*/
static void pirateCallback(uint8_t frame, uint8_t *data) {
  uint8_t i = random(NUMBER_LEDS);
  leds.setPixelColor(i, 255, 255, 255);
  leds.show();
  deepSleep(40);
  leds.setPixelColor(i, 255, 200, 0);
  leds.show();
}

/**
   Waving flag pirate animation
*/
void pirate() {
  playAnimation(PIRATE_address, 60, &pirateCallback);
}

/**
   Per-frame callback for rainbow
*/
static void rainbowCallback(uint8_t frame, uint8_t *data) {
  //Unpack the data
  float hue = (float) * data / 100.0;

  //Define rows of leds
  uint8_t rows[4][2] = {
    {7, 6},
    {0, 5},
    {1, 4},
    {2, 3}
  };

  for (uint8_t row = 0; row < 4; row++) {
    float rowhue = hue + (row * .25);
    if (rowhue >= 1) rowhue--;

    uint32_t color = HSVtoRGB(rowhue, 1, 1);
    for (uint8_t i = 0; i < 2; i++) {
      leds.setPixelColor(rows[row][i], color);
    }

    //Increment row and color and loop around
    hue += .01;
    if (hue >= 1) hue = 0;
  }

  leds.show();

  //Pack the data and store for next time
  *data = (int)(hue * 100);
}

/**
   Rainbow animation, rows of RGBIV
*/
void rainbow() {
  playAnimation(0x0, 70, &rainbowCallback);
}

/**
   Per-frame callback for the remember me animation
*/
static void rememberMeCallback(uint8_t frame, uint8_t *data) {
  uint32_t colors[] = {0xFFFFFFFF, roygbiv[0], roygbiv[0], roygbiv[0], roygbiv[1], roygbiv[1], roygbiv[1], roygbiv[2], roygbiv[2], roygbiv[2]};
  if (frame >= 18 && frame <= 27) {
    setAllLeds(colors[frame - 18]);
    safeDisplay();
  } else {
    ledsOff();
  }
}

/**
   Bender remember me animation
*/
void rememberme() {
  playAnimation(REMEMBER_ME_address, 100, &rememberMeCallback);
}

/**
   Do rick roll animation
*/
void rick() {
  playAnimation(RICK_address, 100, NULL);
}

/**
   Helper function that does all the dirty work of converting some text and special BMP parameters into
   an array of bitmaps that can be scrolled
   This is broken out as a seperate function since it needs to be called each time the scroll changes. Other option
   is to call scroll from within scroll and that's not a good idea to recurse
*/
static void _scrollTextToBmp(uint32_t specialBmp, char *text, bitmap *bmps, uint16_t *width, uint8_t *bmpCount) {
  //Array of addresses for the font - these map 1:1 with INPUT_CHARS string
  uint32_t purisaFont[] = {
    PURISA_SPACE, PURISA_A, PURISA_B, PURISA_C, PURISA_D,
    PURISA_E, PURISA_F, PURISA_G, PURISA_H, PURISA_I,
    PURISA_J, PURISA_K, PURISA_L, PURISA_M, PURISA_N,
    PURISA_O, PURISA_P, PURISA_Q, PURISA_R, PURISA_S,
    PURISA_T, PURISA_U, PURISA_V, PURISA_W, PURISA_X,
    PURISA_Y, PURISA_Z, PURISA_0, PURISA_1, PURISA_2,
    PURISA_3, PURISA_4, PURISA_5, PURISA_6, PURISA_7,
    PURISA_8, PURISA_9, PURISA_PERIOD, PURISA_EXCLAMATION, PURISA_QUESTION,
    PURISA_COMMA, PURISA_LPAREN, PURISA_RPAREN, PURISA_LBRACKET, PURISA_RBRACKET,
    PURISA_LBRACE, PURISA_RBRACE, PURISA_LT, PURISA_GT, PURISA_FORWARD,
    PURISA_BACKWARD, PURISA_PIPE, PURISA_SEMI, PURISA_COLON
  };

  *width = 0;
  *bmpCount = strlen(text);

  //If a special bitmap was specified move the text over by one
  uint8_t bmpIndex = 0;
  if (specialBmp > 0x0) {
    *bmpCount += 2;
    bmpIndex = 1;
    bmps[0] = getBitmapMetadata(specialBmp);
    bmps[*bmpCount - 1] = bmps[0];
    *width += (2 * bmps[0].width);
  }

  //Load bitmap metadata for each character
  for (uint8_t i = 0;  i < strlen(text); i++) {
    int8_t index = ANXIndexOf(INPUT_CHARS, text[i]);
    if (index >= 0 && index < INPUT_CHARS_COUNT) {
      bmps[bmpIndex] = getBitmapMetadata(purisaFont[index]);
      *width += bmps[bmpIndex].width;
      bmpIndex++;
    }
  }
}

/**
   Scrolling text animation customizable by the user
   text - Pointer to existing text (up to 16 characters) to scroll *MUST BE UPPER CASE LOOK AND IN INPUT_CHAR STRING*
   specialBmp - address of BMP to add the scroll, or 0x0 if nothing
   allowEdit - allow the user to edit the scroll text?
   Returns -1 if exited by user (BACK)
*/
int8_t scroll(char *text, uint32_t specialBmp, bool allowEdit, bool loop) {
  bitmap bmps[18];
  //Array of pointers used to dynamically allocate memory for bmps as they enter the screen and freed
  //when they leave
  uint8_t *bmpPixels[18];
  uint8_t bmpCount = 0;

  //Reset the bmp pointers to 0
  for (uint8_t i = 0; i < 18; i++) {
    bmpPixels[i] = (uint32_t)0x0;
  }

  //Init screen position variables
  int16_t x = display.width();
  uint16_t width = 0; //We'll figure this out later

  _scrollTextToBmp(specialBmp, text, bmps, &width, &bmpCount);

  //Scroll until they stop us
  while (1) {
    display.clearDisplay();

    //Try to draw each bitmap (if it even appears on screen at all)
    int16_t tempX = x;  //temporary x variable use for each bmp draw
    for (uint8_t i = 0; i < bmpCount && tempX < display.width(); i++) {
      bitmap bmp = bmps[i];

      //Draw the bitmaps
      if (bmp.address > 0x0) {


        //Only draw the bitmap if it's on screen
        if (tempX + bmp.width >= 0) {
          if ((uint32_t)bmpPixels[i] == 0x0) {

            uint16_t s = ceil(bmp.width * bmp.height / 8);
            bmpPixels[i] = (uint8_t *)malloc(s);
            flash.readBytes(bmp.address, bmpPixels[i], s);
          }

          drawBitmap(bmp, bmpPixels[i], tempX, 0, WHITE);
        }

        //If it fell off the screen, free the data from ram
        else {
          if ((uint32_t)bmpPixels[i] >= 0x20000000) {
            free(bmpPixels[i]);
            bmpPixels[i] = (uint32_t)0x0;
          }
        }

        tempX += bmp.width + 1; //Move temporary X
      }
    }

    safeDisplay();

    //Move verything a little bit
    x -= 3;
    if (x < (0 - width)) {
      x = display.width();
      if (!loop) {
        //Free all memory being used for the scroll
        for (uint8_t i = 0; i < 18; i++) {
          if ((uint32_t)bmpPixels[i] >= 0x20000000) {
            free(bmpPixels[i]);
            bmpPixels[i] = (uint32_t)0x0;
          }
        }
        //Return normal
        return 0;
      }
    }

    deepSleep(12);

    //Handle tasks
    tick();

    //Handle some input
    uint8_t button = getButtonState();
    if (button > 0) {
      safeClearButtonState();
      if (button == BUTTON_LEFT) {

        //Free all memory being used for the scroll
        for (uint8_t i = 0; i < 18; i++) {
          if ((uint32_t)bmpPixels[i] >= 0x20000000) {
            free(bmpPixels[i]);
            bmpPixels[i] = (uint32_t)0x0;
          }
        }

        ledsOff();
        return -1;
      }

      if (button == BUTTON_ENTER && allowEdit) {
        display.setTextSize(1);

        //Possibly clear the existing message
        if (yesNoDialog("Clear existing message?")) {
          memset(text, '\0', 16);
        }

        //Update the text
        disablePopups();
        ANXInputWindow(text, "Message", 16);
        enablePopups();

        //Check for special phrases
        if (strcmp(text, "JACKSON") == 0) {
          addExperience(XP_PER_LEVEL);
          statusDialog("Thank you for\nyour support.");
          ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_EFF);
          safeWaitForButton();
        }
        //Hackaday unlock
        else if (strcmp(text, "SZCZYS") == 0) {
          addExperience(XP_PER_LEVEL);
          statusDialog("Fresh Hacks\nEvery Day\nUnlocked");
          ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_HACKADAY);
          safeWaitForButton();
        }
        //Pirate unlock
        else if (strcmp(text, "YARR") == 0) {
          addExperience(XP_PER_LEVEL);
          statusDialog("Whiskey Unlocked");
          ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_PIRATES);
          safeWaitForButton();
        }
        //Special thank you
        else if (strcmp(text, "TRUE") == 0) {
          statusDialog("Thank you\ntrue!");
          safeWaitForButton();
        }

        //Confirm scroll change with user
        char buffer[40];
        sprintf(buffer, "Change scroll\nto: %s?", text);
        if (yesNoDialog(buffer)) {
          specialBmp = 0x0;

          //Add DEFCON skull to special people
          if (strcmp(text, "AND!XOR") == 0 ||
              strcmp(text, "ANDREW") == 0 ||
              strcmp(text, "JORGE") == 0) {
            specialBmp = DEFCON_address;
          }

          //Store message to flash
          ANXSetScrollText(text);

          //Free all memory that may have been used for the scroll
          for (uint8_t i = 0; i < 18; i++) {
            if ((uint32_t)bmpPixels[i] >= 0x20000000) {
              free(bmpPixels[i]);
              bmpPixels[i] = (uint32_t)0x0;
            }
          }

          //Update all the scroll parameters, bmps, width, etc
          _scrollTextToBmp(specialBmp, text, bmps, &width, &bmpCount);
        }
      }
    }
  }
}

/**
   Scroll text from the user
*/
void scrollingText() {
  //Get scroll text from settings
  char text[16];
  memset(text, '\0', 16);
  ANXGetScrollText(text);
  scroll(text, 0x0, true, true);
}

/**
   Snake animation on LEDs and Screen
*/
void snake() {

  //Clear the screen
  display.clearDisplay();
  safeDisplay();

  //LED
  uint32_t lastLed = 0;
  uint8_t ledDelay = 50;
  float hue = 0;
  int8_t index;
  uint8_t ledOrder[] = {7, 0, 1, 2, 3, 4, 5, 6};

  //Snake on the screen data
  struct coords {
    int16_t x, y;
  };
  uint8_t snakeLen = 40;
  uint8_t snakeI = 0;
  coords snake[snakeLen];
  int8_t snakeX = 0, snakeY = 0;
  uint8_t snakeCounter = 0;
  bool hitAWall = false;

  //Pick a direction to start in
  switch (random(4)) {
    case 0:
      snakeX = 1;
      break;
    case 1:
      snakeX = -1;
      break;
    case 2:
      snakeY = 1;
      break;
    case 3:
      snakeY = 1;
      break;
  }

  //Init the snake
  coords center;
  center.x = display.width() / 2;
  center.y = display.height() / 2;
  for (uint8_t i = 0; i < snakeLen; i++) {
    snake[i] = center;
  }

  while (1) {

    //Bling the LEDs
    if ((rtMillis() - lastLed) > ledDelay) {
      lastLed = rtMillis();
      leds.setPixelColor(ledOrder[index], HSVtoRGB(hue, 1, 1));
      leds.show();
      index++;

      if (index >= NUMBER_LEDS) {
        index = 0;
        hue += .08;
        if (hue >= 1) hue = 0;
      }

      //Handle tasks
      tick();
    }

    //Start the snake on the screen
    hitAWall = false;
    coords *tail = &snake[(snakeI + 1) % snakeLen];
    coords *curr = &snake[snakeI];
    coords *next = &snake[(snakeI + 1) % snakeLen];

    for (uint8_t i = 0; i < snakeLen; i++) {
      display.drawPixel(snake[(snakeI + i) % snakeLen].x, snake[(snakeI + i) % snakeLen].y, WHITE);
    }
    display.drawPixel(tail->x, tail->y, BLACK);

    safeDisplay();

    //Wall detection
    if (snakeX != 0) {
      if (curr->x == 0 || curr->x == display.width() - 1) {
        snakeX = 0;
        if (curr->y == 0) snakeY = 1;
        else if (curr->y == display.height() - 1) snakeY = -1;
        else snakeY = random(2) - 1;
        snakeCounter = 0;
        hitAWall = true;
      }
    }

    //Wall detection
    if (snakeY != 0) {
      if (curr->y == 0 || curr->y == display.height() - 1) {
        snakeY = 0;
        if (curr->x == 0) snakeX = 1;
        else if (curr->x == display.width() - 1) snakeX = -1;
        else snakeX = random(2) - 1;
        snakeCounter = 0;
        hitAWall = true;
      }
    }

    //Randomly change directions
    if (random(20) == 0 && snakeCounter >= 5 && !hitAWall) {
      uint8_t r = random(4);
      if (r == 0 && snakeX != -1) {
        snakeX = 1;
        snakeY = 0;
        snakeCounter = 0;
      } else if (r == 1 && snakeX != 1) {
        snakeX = -1;
        snakeY = 0;
        snakeCounter = 0;
      } else if (r == 2 && snakeY != -1) {
        snakeX = 0;
        snakeY = 1;
        snakeCounter = 0;
      } else if (r == 3 && snakeY != 1) {
        snakeX = 0;
        snakeY = -1;
        snakeCounter = 0;
      }
    }

    //Move the snake one pixel
    next->x = curr->x + snakeX;
    next->y = curr->y + snakeY;
    if (next->x <= 0) next->x = 0;
    if (next->y <= 0) next->y = 0;
    if (next->x >= display.width() - 1) next->x = display.width() - 1;
    if (next->y >= display.height() - 1) next->y = display.height() - 1;
    snakeCounter++;

    //advance the pointer
    snakeI = (snakeI + 1) % snakeLen;

    deepSleep(20);

    //Handle some input
    uint8_t button = getButtonState();
    if (button > 0) {
      safeClearButtonState();
      if (button == BUTTON_LEFT) {
        ledsOff();
        break;
      }
    }
  }
}

/**
   Display status of unlocks
*/
void unlockStatus() {
  ledsOff();

  //Light up one pixel per unlock
  for (uint8_t i = 0; i < 8; i++) {
    if ((ANXGetUnlocked() & (1 << i)) > 0) {
      leds.setPixelColor(i, roygbiv[i]);
    }
  }

  //Turn on the last LED if they unlocked everything
  if (ANXGetUnlocked() == 0x7F) {
    leds.setPixelColor(7, 255, 255, 255);
  }

  leds.show();

  //Play AND!XOR ripple animation until user stops it
  playAnimation(ANDnXOR_RIPPLE_address, 100, NULL, true);

  //As you were
  display.clearDisplay();
  safeDisplay();
}

/**
   War games animation
*/
void warGames() {
  char greetings[] = "GREETINGS PROFESSOR\nFALKEN   \n\nHELLO     \n\nA STRANGE GAME.\nTHE ONLY WINNING\nMOVE IS NOT TO PLAY.";

  while (1) {
    setEyeLeftLed(255, 255, 0);
    setEyeRightLed(255, 255, 0);

    display.clearDisplay();
    display.setCursor(0, 0);
    for (uint8_t i = 0; i < strlen(greetings); i++) {
      display.print(greetings[i]);
      safeDisplay();
      deepSleep(200);
      tick();

      //Handle some input
      uint8_t button = getButtonState();
      if (button > 0) {
        safeClearButtonState();
        return;
      }
    }

    setAllLeds(roygbiv[0]);
    playAnimation(WARGAMES_address, 100, NULL, false);
    ledsOff();
  }
}

