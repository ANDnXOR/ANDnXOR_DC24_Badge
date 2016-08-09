#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Arduino.h>
#include <SPI.h>

#include "ANX.h"
#include "buttons.h"
#include "flash.h"
#include "graphics.h"
#include "serial.h"
#include "term.h"

extern Adafruit_NeoPixel  leds;
extern ANXFlash flash;

/**
   Primary entry point for serial mode
*/
void doSerial() {
  noInterrupts();
  uint32_t lastReadyTime = 0;
  statusDialog("Serial Mode\n'c' for term\n8/N/1 115200");
  bool left = true;
  uint8_t leftCol[] = {7, 0, 1, 2};
  uint8_t rightCol[] = {3, 4, 5, 6};
  uint8_t index = 0;
  uint8_t colorIndex = 0;
  uint32_t color = roygbiv[colorIndex]; //green
  while (1) {

    //Run in serial mode until a button is pressed
    while (getButtonState() == 0) {

      //Only do stuff every 500ms
      if (rtMillis() - lastReadyTime > 500) {
        Serial.println("Ready");
        if (Serial.available()) {
          break;
        }

        lastReadyTime = rtMillis();
      }

      leds.setPixelColor(leftCol[index], 255, 255, 255);
      leds.setPixelColor(rightCol[index], 255, 255, 255);
      leds.show();
      deepSleep(40);

      leds.setPixelColor(leftCol[index], color);
      leds.setPixelColor(rightCol[index], color);
      leds.show();

      index++;
      if (index == 4) {
        index = 0;
        colorIndex = (colorIndex + 1) % 7;
        color = roygbiv[colorIndex];
      }

      deepSleep(100);
      tick();
    }

    //If we stopped due to button, don't execute any commands
    if (getButtonState() > 0) {
      clearButtonState();
      ledsOff();
      interrupts();
      return;
    }

    uint8_t command = Serial.read();

    if (command == 'w') {
      _writeCommand();
    } else if (command == 's') {
      _readSerialCommand();
    } else if (command == 'q') {
      return;
    } else if (command == 'c') {
      terminalMode();
      Serial.println("I’m Bender, baby! Oh god, please insert liquor!\n");
      return;
    } else {
      Serial.print("Got invalid command: 0x"); Serial.println(command, HEX);
    }
  }
}

/**
   Dump remaining incoming data from serial
*/
void flushIncoming() {
  while (Serial.available() > 0) {
    char t = Serial.read();
  }
}

/**
   Read raw data from serial up to a certain size
*/
void readDataFromSerial(uint8_t *buffer, int size) {
  memset(buffer, '\0', 4096);
  uint16_t bytesReceived = 0;
  uint16_t counter = 0;

  while (bytesReceived < size) {
    //Read the byte
    if (Serial.available()) {
      int16_t b = Serial.read();
      buffer[bytesReceived] = b;
      bytesReceived++;
      //ACK
      Serial.println("A");
    }
  }
}

/**
   Read a line from serial. Stops at '\n'
*/
void readLineFromSerial(char *buffer) {
  memset(buffer, '\0', 256);
  uint8_t i = 0;
  int16_t c = Serial.read();
  while (c != '\n') {
    if (c >= 32 && c <= 126) {
      buffer[i] = c;
      i++;
    }
    c = Serial.read();
  }
}

/**
   Read the serial number of the MCU
*/
void _readSerialCommand() {
  Serial.println("SERIAL"); //Print marker indicating next line is serial
  unsigned long *unique = (unsigned long *)0x1FFFF7E8;
  Serial.print(unique[0], HEX); Serial.print(":");
  Serial.print(unique[1], HEX); Serial.print(":");
  Serial.println(unique[2], HEX);
  Serial.println("Done");
}

/**
   Bulk write to the SPI flash chip, this will erase 4k at a time so be careful
*/
void _writeCommand() {
  noInterrupts();
  ledsOff();

  //Reserve some memory
  char charBuffer[256];
  uint8_t byteBuffer[4096];

  flushIncoming();
  Serial.println("Address:");
  readLineFromSerial(charBuffer);
  Serial.println(charBuffer);
  int address = atol(charBuffer);

  flushIncoming();
  Serial.println("Size(Bytes):");
  readLineFromSerial(charBuffer);
  int size = atol(charBuffer);

  //Ready to go
  flushIncoming();
  Serial.println("OK GO");

  uint32_t bytesLeft = size;
  uint32_t tempAddress = address;
  while (bytesLeft > 0) {
    uint16_t s = min(bytesLeft, 4096);

    readDataFromSerial(byteBuffer, s);

    //Erase the 4k block before writing to it
    leds.setPixelColor(0, roygbiv[0]);
    leds.setPixelColor(5, roygbiv[0]);
    leds.show();
    flash.erase4k(tempAddress);

    //Write to it
    leds.setPixelColor(0, roygbiv[3]);
    leds.setPixelColor(5, roygbiv[3]);
    leds.show();
    flash.writeBytes(tempAddress, byteBuffer, s);

    //Move the flash pointer
    tempAddress += s;
    bytesLeft -= s;

    Serial.println(bytesLeft);
  }

  //Clear the buffer
  memset(byteBuffer, '\0', 4096);

  Serial.println("\nDONE");
  interrupts();

  ledsOff();
}
