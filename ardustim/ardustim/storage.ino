#include "storage.h"
#include "EEPROM.h"
#include "wheel_defs.h"
#include "ardustim.h"
#include "enums.h"
#include "globals.h"
#if defined(ARDUSTIM_F401) || defined(ARDUSTIM_ESP32C3)
#include "pattern_backend.h"

// Buffer the entire save and erase/program flash only once if anything changed.
static bool storageChanged = false;
static void updateStorage(int address, uint8_t value)
{
#if defined(ARDUSTIM_ESP32C3)
  if (EEPROM.read(address) != value)
#else
  if (eeprom_buffered_read_byte(address) != value)
#endif
  {
#if defined(ARDUSTIM_ESP32C3)
    EEPROM.write(address, value);
#else
    eeprom_buffered_write_byte(address, value);
#endif
    storageChanged = true;
  }
}
#else
static void updateStorage(int address, uint8_t value)
{
  EEPROM.update(address, value);
}
#endif

void loadConfig()
{
  config.version = VERSION;
  if(EEPROM.read(EEPROM_VERSION) == 255)
  {
    //New arduino
    config.wheel = 5; //36-1
    currentStatus.rpm = 3000;
    currentStatus.base_rpm = 3000;
    config.mode = POT_RPM;

    config.fixed_rpm = 3500;
    config.sweep_high_rpm = 6000;
    config.sweep_low_rpm = 1000;
    config.sweep_interval = 1000;

    config.useCompression = false;
    config.compressionType = COMPRESSION_TYPE_4CYL_4STROKE;
    config.compressionRPM = 400;
    config.compressionOffset = 0;

    saveConfig();
  }
  else
  {
    config.wheel = EEPROM.read(EEPROM_WHEEL);
    config.mode = EEPROM.read(EEPROM_RPM_MODE);

    byte highByte = EEPROM.read(EEPROM_CURRENT_RPM);
    byte lowByte =  EEPROM.read(EEPROM_CURRENT_RPM+1);
    currentStatus.rpm = word(highByte, lowByte);

    highByte = EEPROM.read(EEPROM_FIXED_RPM);
    lowByte =  EEPROM.read(EEPROM_FIXED_RPM+1);
    config.fixed_rpm = word(highByte, lowByte);
    config.fixed_rpm = constrain(config.fixed_rpm, 100, TMP_RPM_CAP);

    highByte = EEPROM.read(EEPROM_SWEEP_RPM_MIN);
    lowByte =  EEPROM.read(EEPROM_SWEEP_RPM_MIN+1);
    config.sweep_low_rpm = word(highByte, lowByte);
    config.sweep_low_rpm = constrain(config.sweep_low_rpm, 100, TMP_RPM_CAP);

    highByte = EEPROM.read(EEPROM_SWEEP_RPM_MAX);
    lowByte =  EEPROM.read(EEPROM_SWEEP_RPM_MAX+1);
    config.sweep_high_rpm = word(highByte, lowByte);
    config.sweep_high_rpm = constrain(config.sweep_high_rpm, 100, TMP_RPM_CAP);

    highByte = EEPROM.read(EEPROM_SWEEP_RPM_INT);
    lowByte =  EEPROM.read(EEPROM_SWEEP_RPM_INT+1);
    config.sweep_interval = word(highByte, lowByte);
    config.sweep_interval = constrain(config.sweep_interval, 200, 10000);

    if(config.sweep_low_rpm >= config.sweep_high_rpm) { config.sweep_low_rpm = config.sweep_high_rpm - 100; }

    config.useCompression = EEPROM.read(EEPROM_USE_COMPRESSION);
    config.compressionType = EEPROM.read(EEPROM_COMPRESSION_TYPE);
    highByte = EEPROM.read(EEPROM_COMPRESSION_RPM);
    lowByte = EEPROM.read(EEPROM_COMPRESSION_RPM+1);
    config.compressionRPM = word(highByte, lowByte);
    highByte = EEPROM.read(EEPROM_COMPRESSION_OFFSET);
    lowByte = EEPROM.read(EEPROM_COMPRESSION_OFFSET+1);
    config.compressionOffset = word(highByte, lowByte);
    //config.compressionType = COMPRESSION_TYPE_6CYL_4STROKE;

    //Error checking
    if(config.wheel >= MAX_WHEELS) { config.wheel = 5; }
    if(config.mode >= MAX_MODES) { config.mode = FIXED_RPM; }
    if(currentStatus.rpm > 15000) { currentStatus.rpm = 4000; }
    if(currentStatus.base_rpm > 15000) { currentStatus.base_rpm = 4000; }
    if(config.compressionType > COMPRESSION_TYPE_8CYL_4STROKE) { config.compressionType = COMPRESSION_TYPE_4CYL_4STROKE; }
    if(config.compressionRPM > 1000) { config.compressionRPM = 400; }
    if(config.compressionOffset > 359) { config.compressionOffset = 0; }
    // Older AVR firmware left this byte erased; only 1 means enabled.
    config.compressionDynamic = EEPROM.read(EEPROM_COMPRESSION_DYNAMIC) == 1;
  }
#if defined(ARDUSTIM_F401) || defined(ARDUSTIM_ESP32C3)
  currentStatus.base_rpm = currentStatus.rpm;
#endif
}

void saveConfig()
{
#if defined(ARDUSTIM_F401)
  eeprom_buffer_fill();
  storageChanged = false;
#endif
  updateStorage(EEPROM_WHEEL, config.wheel);
  updateStorage(EEPROM_RPM_MODE, config.mode);
  updateStorage(EEPROM_VERSION, VERSION);

  byte highByte = highByte(currentStatus.rpm);
  byte lowByte = lowByte(currentStatus.rpm);
  updateStorage(EEPROM_CURRENT_RPM, highByte);
  updateStorage(EEPROM_CURRENT_RPM+1, lowByte);

  highByte = highByte(config.fixed_rpm);
  lowByte = lowByte(config.fixed_rpm);
  updateStorage(EEPROM_FIXED_RPM, highByte);
  updateStorage(EEPROM_FIXED_RPM+1, lowByte);

  highByte = highByte(config.sweep_low_rpm);
  lowByte = lowByte(config.sweep_low_rpm);
  updateStorage(EEPROM_SWEEP_RPM_MIN, highByte);
  updateStorage(EEPROM_SWEEP_RPM_MIN+1, lowByte);

  highByte = highByte(config.sweep_high_rpm);
  lowByte = lowByte(config.sweep_high_rpm);
  updateStorage(EEPROM_SWEEP_RPM_MAX, highByte);
  updateStorage(EEPROM_SWEEP_RPM_MAX+1, lowByte);

  highByte = highByte(config.sweep_interval);
  lowByte = lowByte(config.sweep_interval);
  updateStorage(EEPROM_SWEEP_RPM_INT, highByte);
  updateStorage(EEPROM_SWEEP_RPM_INT+1, lowByte);

  updateStorage(EEPROM_USE_COMPRESSION, config.useCompression);
  updateStorage(EEPROM_COMPRESSION_TYPE, config.compressionType);
  highByte = highByte(config.compressionRPM);
  lowByte = lowByte(config.compressionRPM);
  updateStorage(EEPROM_COMPRESSION_RPM, highByte);
  updateStorage(EEPROM_COMPRESSION_RPM+1, lowByte);
  highByte = highByte(config.compressionOffset);
  lowByte = lowByte(config.compressionOffset);
  updateStorage(EEPROM_COMPRESSION_OFFSET, highByte);
  updateStorage(EEPROM_COMPRESSION_OFFSET+1, lowByte);
  updateStorage(EEPROM_COMPRESSION_DYNAMIC, config.compressionDynamic);
#if defined(ARDUSTIM_F401) || defined(ARDUSTIM_ESP32C3)
  if (storageChanged)
  {
    backendBeginSave();
#if defined(ARDUSTIM_ESP32C3)
    // Keep dirty RAM contents on failure; a later Save retries the commit.
    storageChanged = !EEPROM.commit();
#else
    eeprom_buffer_flush();
#endif
    backendEndSave();
  }
#endif
}
