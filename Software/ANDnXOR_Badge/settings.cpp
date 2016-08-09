#include "ANX.h"
#include "flash.h"
#include "rf.h"
#include "serial.h"
#include "settings.h"

extern ANXFlash    flash;
static byte temp[SETTINGS_SIZE];

//Local storage of some settings to reduce load on flash
//These follow a write-through cache pattern
char username[USERNAME_MAX_LENGTH + 1];
bool airplane = AIRPLANE_DEFAULT;
bool alert = ALERT_DEFAULT;
bool eyes = EYES_DEFAULT;
uint8_t level = LEVEL_DEFAULT;
bool tilt = TILT_DEFAULT;
byte unlocked = UNLOCKED_DEFAULT;

/**
   Get setting for if alerts are enabled
*/
bool ANXGetAlert() {
  return alert;
}

/**
   Set alerts enabled settings
*/
void ANXSetAlert(bool value) {
  alert = value;
  modifySetting(ALERT_ADDRESS, value, true);
}


/**
   Get the Airplane mode on/off setting
*/
bool ANXGetAirplane() {
  return airplane;
}

/**
   Set the airplane mode on/off setting
*/
void ANXSetAirplane(bool value) {
  if (value) {
    ANXRFSleep();
  }
  modifySetting(AIRPLANE_ADDRESS, value, true);
  airplane = value;
}

/**
   Get the experience value
*/
uint32_t ANXGetExperience() {
  uint32_t xp;
  getSetting(EXPERIENCE_ADDRESS, (char *)(&xp), 4);
  if (xp >= XP_PER_LEVEL) {
    ANXSetExperience(EXPERIENCE_DEFAULT);
    return EXPERIENCE_DEFAULT;
  }
  return xp;
}

/**
   Set the experience value
*/
void ANXSetExperience(uint32_t value) {
  modifySetting(EXPERIENCE_ADDRESS, (char *)(&value), 4, true);
}

/**
   Get LED eyes enabled setting
*/
bool ANXGetEyes() {
  return eyes;
}

/**
   Set eyes enabled setting
*/
void ANXSetEyes(bool value) {
  modifySetting(EYES_ADDRESS, value, true);
  eyes = value;
}

/**
   Get the player's level
*/
uint8_t ANXGetLevel() {
  return level;
}

/**
   Set the player's level
*/
void ANXSetLevel(uint8_t value) {
  modifySetting(LEVEL_ADDRESS, value, true);
  level = value;
}

/**
   Sets the scroll in the settings
*/
void ANXSetScrollText(char *scroll) {
  modifySetting(SCROLLING_ADDRESS, scroll, 16, true);
}

/**
   Gets the current scroll text into the scroll variable. If scroll text not in settings
   initialize to default
*/
void ANXGetScrollText(char *scroll) {
  getSetting(SCROLLING_ADDRESS, scroll, 16);
  if (scroll[0] < 20 || scroll[0] > 128) {
    ANXSetScrollText(SCROLLING_DEFAULT);
    sprintf(scroll, SCROLLING_DEFAULT);
  }
}

/**
   Gets sudo setting, if not in settings default is returned
*/
bool ANXGetSudo() {
  byte b = getSetting(SUDO_ADDRESS);
  if (b > 1) {
    ANXSetSudo(SUDO_DEFAULT);
    return SUDO_DEFAULT;
  }
  return b;
}

/**
   Set the sudo value
*/
void ANXSetSudo(bool value) {
  modifySetting(SUDO_ADDRESS, value, true);
}

/**
   Gets swear mode setting, if not in settings default is returned
*/
bool ANXGetSwearMode() {
  byte b = getSetting(SWEARMODE_ADDRESS);
  if (b > 1) {
    ANXSetSwearMode(SWEARMODE_DEFAULT);
    return SWEARMODE_DEFAULT;
  }
  return (b == 1);
}

/**
   Set the swear mode value
*/
void ANXSetSwearMode(bool value) {
  modifySetting(SWEARMODE_ADDRESS, value, true);
}

/**
   Get if tilt is enabled
*/
bool ANXGetTilt() {
  return tilt;
}

/**
   Set tilt enabled setting
*/
void ANXSetTilt(bool value) {
  modifySetting(TILT_ADDRESS, value, true);
  tilt = value;
}

/**
   Gets unlocked setting, if not in settings default is returned
*/
byte ANXGetUnlocked() {
  return unlocked;
}

/**
   Set the unlocked value
*/
void ANXSetUnlocked(byte value) {
  unlocked = value;
  modifySetting(UNLOCKED_ADDRESS, value, true);
}

/**
   Reset to factory defaults
*/
void resetSettings() {
  ANXSetAirplane(AIRPLANE_DEFAULT);
  ANXSetAlert(ALERT_DEFAULT);
  ANXSetExperience(EXPERIENCE_DEFAULT);
  ANXSetEyes(EYES_DEFAULT);
  ANXSetLevel(LEVEL_DEFAULT);
  ANXSetScrollText(SCROLLING_DEFAULT);
  ANXSetSudo(SUDO_DEFAULT);
  ANXSetTilt(TILT_DEFAULT);
  ANXSetUnlocked(UNLOCKED_DEFAULT);
  ANXSetUsername(USERNAME_DEFAULT);
}


/**
   Gets the current username into the username variable. If Username not in settings
   initialize to default
*/
void ANXGetUsername(char *uname) {
  memcpy(uname, username, USERNAME_MAX_LENGTH);
}

/**
   Sets the username in the settings
*/
void ANXSetUsername(char *uname) {
  modifySetting(USERNAME_ADDRESS, uname, USERNAME_MAX_LENGTH, true);
  memcpy(username, uname, USERNAME_MAX_LENGTH);
}

/**
   Get a single byte setting
*/
byte getSetting(uint8_t address) {
  return flash.readByte(SETTINGS_ADDRESS + address);
}

/**
   Get a setting that's more than one byte
*/
void getSetting(uint8_t address, char *bytes, uint16_t size) {
  flash.readBytes((SETTINGS_ADDRESS + address), bytes, size);
}

/**
   Initialize the settings, must be called before first use
*/
void initSettings() {
  //Init the username
  //Clear the current local storage
  memset(username, '\0', USERNAME_MAX_LENGTH + 1);
  //Read from flash
  getSetting(USERNAME_ADDRESS, username, USERNAME_MAX_LENGTH);
  //If it's invalid (like from clear NAND), overwrite with default value
  if (username[0] < 20 || username[0] > 128) {
    modifySetting(USERNAME_ADDRESS, USERNAME_DEFAULT, USERNAME_MAX_LENGTH, false);
    memcpy(username, USERNAME_DEFAULT, USERNAME_MAX_LENGTH);
  }

  //Init airplane mode setting
  byte b = getSetting(AIRPLANE_ADDRESS);
  if (b > 1) {
    modifySetting(AIRPLANE_ADDRESS, AIRPLANE_DEFAULT, false);
    b = AIRPLANE_DEFAULT;
  }
  airplane = (b == 1);

  //Init alert
  b = getSetting(ALERT_ADDRESS);
  if (b > 1) {
    modifySetting(ALERT_ADDRESS, ALERT_DEFAULT, false);
    b = ALERT_DEFAULT;
  }
  alert = (b == 1);

  //Init eyes
  b = getSetting(EYES_ADDRESS);
  if (b > 1) {
    modifySetting(EYES_ADDRESS, EYES_DEFAULT, false);
    b = EYES_DEFAULT;
  }
  eyes = (b == 1);

  //Init level
  b = getSetting(LEVEL_ADDRESS);
  if (b == 0xFF) {
    modifySetting(LEVEL_ADDRESS, LEVEL_DEFAULT, false);
    b = LEVEL_DEFAULT;
  }
  level = (uint8_t)b;

  //Init tilt
  b = getSetting(TILT_ADDRESS);
  if (b > 1) {
    modifySetting(TILT_ADDRESS, TILT_DEFAULT, false);
    b = TILT_DEFAULT;
  }
  tilt = (b == 1);

  //Init unlocked
  b = getSetting(UNLOCKED_ADDRESS);
  if (b == 0xFF) {
    modifySetting(UNLOCKED_ADDRESS, UNLOCKED_DEFAULT, true); //commit on last setting
    b = UNLOCKED_DEFAULT;
  }
  unlocked = b;
}

/**
   Modify a one byte setting in cache
   Set commit to true to immediately write to NAND. This should be done anytime the setting is
   changed by the user. Use false to batch changes to NAND and save some power
*/
void modifySetting(uint8_t address, byte b, bool commit) {
  flash.readBytes(SETTINGS_ADDRESS, temp, SETTINGS_SIZE);

  temp[address] = b;

  if (commit) {
    flash.erase4k(0x0);
    flash.writeBytes(SETTINGS_ADDRESS, temp, SETTINGS_SIZE);
  }
}

/**
   Modify a multi-byte setting in cache
   Set commit to true to immediately write to NAND. This should be done anytime the setting is
   changed by the user. Use false to batch changes to NAND and save some power
*/
void modifySetting(uint8_t address, char *bytes, uint16_t size, bool commit) {
  flash.readBytes(SETTINGS_ADDRESS, temp, SETTINGS_SIZE);

  for (uint16_t i = 0; i < size; i++) {
    temp[address + i] = bytes[i];
  }

  if (commit) {
    flash.erase4k(0x0);
    flash.writeBytes(SETTINGS_ADDRESS, temp, SETTINGS_SIZE);
  }
}
