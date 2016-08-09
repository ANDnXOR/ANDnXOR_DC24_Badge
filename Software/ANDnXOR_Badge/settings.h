#ifndef ANX_SETTINGS_H
#define ANX_SETTINGS_H

#define SETTINGS_ADDRESS      0xFF

#define USERNAME_ADDRESS      0x1

#define SUDO_DEFAULT          0
#define SUDO_ADDRESS          0x9

#define UNLOCKED_DEFAULT      0
#define UNLOCKED_ADDRESS      0xA

#define AIRPLANE_DEFAULT      0
#define AIRPLANE_ADDRESS      0xB

#define SWEARMODE_DEFAULT     0
#define SWEARMODE_ADDRESS     0xC

#define SCROLLING_DEFAULT     "DEFCON 24"
#define SCROLLING_ADDRESS     0xD   //max length = 16

#define TILT_DEFAULT          1
#define TILT_ADDRESS          0x1D

#define EYES_DEFAULT          1
#define EYES_ADDRESS          0x1E

#define ALERT_DEFAULT         1
#define ALERT_ADDRESS         0x1F

#define LEVEL_DEFAULT         1
#define LEVEL_ADDRESS         0x20
#define EXPERIENCE_DEFAULT    0
#define EXPERIENCE_ADDRESS    0x21  //uint32_t

#define SETTINGS_SIZE         0x24

bool ANXGetAirplane();
void ANXSetAirplane(bool value);

bool ANXGetAlert();
void ANXSetAlert(bool value);

uint32_t ANXGetExperience();
void ANXSetExperience(uint32_t value);

bool ANXGetEyes();
void ANXSetEyes(bool value);

uint8_t ANXGetLevel();
void ANXSetLevel(uint8_t value);

void ANXGetScrollText(char *scroll);
void ANXSetScrollText(char *scroll);

bool ANXGetSudo();
void ANXSetSudo(bool value);

bool ANXGetSwearMode();
void ANXSetSwearMode(bool value);

bool ANXGetTilt();
void ANXSetTilt(bool value);

byte ANXGetUnlocked();
void ANXSetUnlocked(byte value);

void ANXGetUsername(char *username);
void ANXSetUsername(char *username);

void editUsername();
byte getSetting(uint8_t address);
void getSetting(uint8_t address, char *bytes, uint16_t size);
void initSettings();
void modifySetting(uint8_t address, byte b, bool erase);
void modifySetting(uint8_t address, char *bytes, uint16_t size, bool erase);
void resetSettings();

#endif
