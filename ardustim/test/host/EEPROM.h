#pragma once
#include <stdint.h>
#include <string.h>
extern uint8_t testFlash[8192], testBuffer[8192];
extern int testFlushes;
struct TestEEPROM {
  uint8_t read(int address) { return testFlash[address]; }
  void update(int address, uint8_t value) { testFlash[address] = value; }
};
extern TestEEPROM EEPROM;
inline void eeprom_buffer_fill() { memcpy(testBuffer, testFlash, sizeof(testFlash)); }
inline uint8_t eeprom_buffered_read_byte(int address) { return testBuffer[address]; }
inline void eeprom_buffered_write_byte(int address, uint8_t value) { testBuffer[address] = value; }
inline void eeprom_buffer_flush() { ++testFlushes; memcpy(testFlash, testBuffer, sizeof(testFlash)); }
