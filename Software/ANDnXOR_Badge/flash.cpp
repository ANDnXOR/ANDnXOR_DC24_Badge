#include "flash.h";

ANXFlash::ANXFlash(uint8_t cs, SPIClass *spi) {
  _cs = cs;
  _spi = spi;
}

/**
   Initialize the flash device
*/
bool ANXFlash::begin() {
  _spi->begin();
  pinMode(_cs, OUTPUT);
  _deselect();

  //Wakeup the device
  this->wakeup();

  while (flashBusy());

  //Clear status registers and any write protection
  _command(FLASH_STATUSWRITE, true);
  _transfer(0); //reset status reg 1
  _transfer(0); //reset status reg 2
  _deselect();
  while (flashBusy());

  //Clear status registers a second time in case registers were write protected
  _command(FLASH_STATUSWRITE, true);
  _transfer(0); //reset status reg 1
  _transfer(0); //reset status reg 2
  _deselect();
  while (flashBusy());
  deepSleep(200);
  this->sleep();
}

/**
   Perform a full chip erase
*/
void ANXFlash::chipErase() {
  this->wakeup();
  _command(FLASH_CHIP_ERASE, true);
  _deselect();
  while (flashBusy());
  this->sleep();
}

/**
   Erase 4k sector
*/
void ANXFlash::erase4k(uint32_t address) {
  this->wakeup();
  _command(FLASH_BLOCKERASE_4K, true);
  _transfer(address >> 16);
  _transfer(address >> 8);
  _transfer(address);
  _deselect();

  while (flashBusy());
  this->sleep();
}

/**
   Erase 64k block
*/
void ANXFlash::erase64k(uint32_t address) {
  this->wakeup();
  _command(FLASH_BLOCKERASE_64K, true);
  _transfer(address >> 16);
  _transfer(address >> 8);
  _transfer(address);
  _deselect();
  _getStatus();
  while (flashBusy());
  this->sleep();
}

/**
   Returns true if flash is busy with a write or erase operation
*/
bool ANXFlash::flashBusy() {
  return _getStatus() & 1;
}

/**
   Read the manufacturer ID (for debugging purposes)
*/
byte ANXFlash::getManufacturerId() {
  this->wakeup();
  _command(FLASH_DEVICE_ID, false);
  byte b = _transfer(0);
  _transfer(0); //dump last two bytes
  _transfer(0);
  this->sleep();

  return b;
}

/**
   Read single byte from the flash memory
*/
byte ANXFlash::readByte(uint32_t address) {
  this->wakeup();
  _command(FLASH_READ, false);
  _transfer(address >> 16);
  _transfer(address >> 8);
  _transfer(address);
  _transfer(0); //don't care, dummy byte required for fast read
  byte b = _transfer(0);

  _deselect();
  this->sleep();
  return b;
}

/**
   Read an arbitrary number of bytes from flash memory
*/
void ANXFlash::readBytes(uint32_t address, void* buffer, uint16_t length) {
  this->wakeup();
  _command(FLASH_READ, false);
  _transfer(address >> 16);
  _transfer(address >> 8);
  _transfer(address);
  _transfer(0); //don't care, dummy byte required for fast read

  for (uint16_t i = 0; i < length; ++i) {
    ((uint8_t*) buffer)[i] = _transfer(0);
  }
  _deselect();
  this->sleep();
}

/**
   Sleep the flash device
*/
void ANXFlash::sleep() {
  _select();
  _transfer(FLASH_SLEEP);
  _deselect();
}

/**
   Wakeup from deep sleep
*/
void ANXFlash::wakeup() {
  _select();
  _transfer(FLASH_RELEASE);
  _deselect();
}

/**
   Write arbitrary length of bytes to an address
*/
void ANXFlash::writeBytes(uint32_t address, const void* buffer, uint16_t length) {
  uint16_t n;
  uint16_t maxBytes = 256 - (address % 256);
  uint16_t offset = 0;

  this->wakeup();

  //Write each byte one at a time
  while (length > 0) {
    n = (length <= maxBytes) ? length : maxBytes;
    _command(FLASH_PAGE_PGM, true);
    _transfer(address >> 16);
    _transfer(address >> 8);
    _transfer(address);

    for (uint16_t i = 0; i < n; i++) {
      _transfer(((uint8_t*) buffer)[offset + i]);
    }
    _deselect();
    while (flashBusy());

    address += n; // adjust the addresses and remaining bytes by what we've just transferred.
    offset += n;
    length -= n;
    maxBytes = 256;   // now we can do up to 256 bytes per loop
  }

  this->sleep();
}

void ANXFlash::_command(uint8_t cmd, bool writeEnable) {
  if (writeEnable) {
    _select();
    _transfer(FLASH_WRITE_ENABLE);
    _deselect();
  }

  delay(1);

  //wait for it to be ready
  while (flashBusy());
  _select();
  _transfer(cmd);
}

byte ANXFlash::_getStatus() {
  _select();
  _transfer(FLASH_STATUS_REG_1);
  uint8_t s = _transfer(0);
  _deselect();
  return s;
}

