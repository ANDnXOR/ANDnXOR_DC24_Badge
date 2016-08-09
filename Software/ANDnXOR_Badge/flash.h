#ifndef FLASH_H
#define FLASH_H

#include <Arduino.h>
#include <SPI.h>
#include "ANX.h"

//COMMANDS
#define FLASH_BLOCKERASE_4K   0x20
#define FLASH_BLOCKERASE_64K  0xD8
#define FLASH_WRITE_ENABLE    0x06  //write enable
#define FLASH_CHIP_ERASE      0xc7  //chip erase
#define FLASH_STATUS_REG_1    0x05
#define FLASH_STATUS_REG_2    0x35
#define FLASH_PAGE_PGM        0x02    //Write Page
#define FLASH_ARRAYREAD       0x0B
#define FLASH_RELEASE         0xAB    //Wakeup
#define FLASH_SLEEP           0xB9    //Sleep
#define FLASH_STATUSWRITE     0x01
#define FLASH_READ            0x0B
#define FLASH_DEVICE_ID       0x9F

class ANXFlash {
  public:

    ANXFlash(uint8_t cs, SPIClass *spi);

    bool begin();
    void chipErase();
    void erase4k(uint32_t address);
    void erase64k(uint32_t address);
    bool flashBusy();
    byte getManufacturerId();
    byte readByte(uint32_t address);
    void readBytes(uint32_t address, void* buffer, uint16_t length);
    void sleep();
    void wakeup();
    void writeBytes(uint32_t address, const void* buffer, uint16_t length) ;

  private:
    uint8_t _cs;
    SPIClass *_spi;
    void _command(uint8_t cmd, bool writeEnable);
    byte _getStatus();
    void _setWriteEnable(bool enable);
    inline void _select() {
      noInterrupts();
      digitalWrite(_cs, LOW);
    }
    inline void _deselect() {
      digitalWrite(_cs, HIGH);
      interrupts();
    }
    inline uint8_t _transfer(uint8_t x) {
      uint8_t y = _spi->transfer(x);
      return y;
    }
};
#endif
