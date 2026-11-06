#pragma once
#include <stdint.h>
#include <string.h>
extern uint8_t testFlash[8192], testBuffer[8192];
extern int testFlushes;
struct TestEEPROM {
#if defined(ARDUSTIM_ESP32C3)
  bool failCommit = false;
  int beginFailures = 0, beginCalls = 0;
  bool begin(int size) {
    ++beginCalls;
    if (size != 32) { return false; }
    if (beginFailures > 0) { --beginFailures; return false; }
    memcpy(testBuffer, testFlash, sizeof(testFlash));
    return true;
  }
  uint8_t read(int address) { return testBuffer[address]; }
  void write(int address, uint8_t value) { testBuffer[address] = value; }
  bool commit() { if (failCommit) { return false; } ++testFlushes; memcpy(testFlash, testBuffer, sizeof(testFlash)); return true; }
#else
  uint8_t read(int address) { return testFlash[address]; }
#endif
  void update(int address, uint8_t value) { testFlash[address] = value; }
};
extern TestEEPROM EEPROM;
inline void eeprom_buffer_fill() { memcpy(testBuffer, testFlash, sizeof(testFlash)); }
inline uint8_t eeprom_buffered_read_byte(int address) { return testBuffer[address]; }
inline void eeprom_buffered_write_byte(int address, uint8_t value) { testBuffer[address] = value; }
inline void eeprom_buffer_flush() { ++testFlushes; memcpy(testFlash, testBuffer, sizeof(testFlash)); }
