#include <cassert>
#include <cstring>
#include <cstdio>
#include "globals.h"
#include "storage.h"
#include "EEPROM.h"
TestEEPROM EEPROM;
uint8_t testFlash[8192], testBuffer[8192];
int testFlushes = 0;
configTable config;
struct status currentStatus;
int main()
{
  memset(testFlash, 255, sizeof(testFlash));
  loadConfig();
  assert(!config.compressionDynamic);
  for (bool enabled : {true, false}) {
    config.compressionDynamic = enabled;
    config.compressionOffset = 359;
    saveConfig();
    config.compressionDynamic = !enabled;
    config.compressionOffset = 0;
    loadConfig();
    assert(config.compressionDynamic == enabled);
    assert(config.compressionOffset == 359);
  }
  testFlash[EEPROM_COMPRESSION_DYNAMIC] = 255; // Pre-fix AVR settings.
  loadConfig();
  assert(!config.compressionDynamic);
  puts("AVR dynamic amplitude storage checks passed.");
}
