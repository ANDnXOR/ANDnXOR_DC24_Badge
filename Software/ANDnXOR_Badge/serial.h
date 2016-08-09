#ifndef SERIAL_H
#define SERIAL_H

extern void doSerial();
extern void flushIncoming();
extern void readDataFromSerial(uint8_t *buffer, int size);
extern void readLineFromSerial(char *buffer);
extern void _eraseCommand();
extern void _readSerialCommand();
extern void _writeCommand();

#endif
