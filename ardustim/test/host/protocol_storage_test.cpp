#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "globals.h"
#include "enums.h"
#include "comms.h"
#include "storage.h"
#include "EEPROM.h"
#include "pattern_timing.h"
#include "pattern_backend.h"

TestSerial Serial;
unsigned long testDelayMilliseconds = 0;
void (*testDelayHook)() = nullptr;
#if defined(ARDUSTIM_ESP32C3)
static void stopAfterTwoMessages() {
  if (testDelayMilliseconds == 2000) { throw 1; }
}
#endif
TestEEPROM EEPROM;
uint8_t testFlash[8192], testBuffer[8192];
int testFlushes = 0, saveBegins = 0, saveEnds = 0, timingUpdates = 0;
int patternStops = 0, patternStarts = 0;
uint8_t wheelAtStop = 0;
bool patternStopped = false;
configTable config;
struct status currentStatus;
wheels Wheels[MAX_WHEELS] = {{"test", nullptr, 1.0f, 4, 360}};
volatile uint16_t edge_counter = 0, new_OCR1A = 0;
volatile uint32_t cycleDuration = 0, cycleStartTime = 0;
volatile uint8_t output_invert_mask = 0;
void reset_new_OCR1A(uint32_t) { ++timingUpdates; }
void backendStop()
{
  wheelAtStop = config.wheel;
  patternStopped = true;
  ++patternStops;
}
void backendStart()
{
  assert(patternStopped);
  patternStopped = false;
  ++patternStarts;
  edge_counter = 0;
  cycleDuration = 0;
  cycleStartTime = micros();
}
#if defined(ARDUSTIM_ESP32C3)
void backendBeginSave() { ++saveBegins; }
void backendEndSave() { ++saveEnds; }
#endif
void f401BeginSave() { ++saveBegins; }
void f401EndSave() { ++saveEnds; }

static void sendConfig(const configTable &value)
{
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&value);
  std::vector<uint8_t> packet = {'c'};
  packet.insert(packet.end(), bytes + 1, bytes + sizeof(value));
  Serial.receive(packet);
  commandParser();
}

int main()
{
  // Exact known intervals and range/rounding across sparse and dense wheels.
  assert(patternPeriodTicks(84000000, 1.0f, 3000) == 14000);
  assert(patternPeriodTicks(84000000, 6.0f, 9000) == 778);
  assert(patternPeriodTicks(84000000, 1.0f, 0) == 4200000);
  for (float scaler : {0.03333f, 0.05f, 0.06667f, 0.6f, 1.0f, 3.0f, 6.0f})
  {
    uint32_t previous = UINT32_MAX;
    for (uint32_t rpm : {10U, 100U, 400U, 1000U, 3000U, 9000U, 15000U})
    {
      const uint32_t ticks = patternPeriodTicks(84000000, scaler, rpm);
      assert(ticks > 0 && ticks < previous);
      assert(std::abs(ticks - 84000000.0 / (2.0 * scaler * rpm)) <= 0.5);
      previous = ticks;
    }
  }
  for (unsigned state = 0; state < 256; ++state)
  {
    const uint32_t bsrr = patternOutputBSRR(state);
    const uint32_t set = bsrr & 0xffffU, reset = bsrr >> 16;
    assert(set == (state & 7U));
    assert((set | reset) == 7U && (set & reset) == 0);
  }

  // Default initialization, one flash transaction, unchanged saves, and reboot.
  memset(testFlash, 255, sizeof(testFlash));
  testFlash[99] = 42; // Unrelated data in the emulated EEPROM must survive.
#if defined(ARDUSTIM_ESP32C3)
  EEPROM.beginFailures = 1;
  testDelayHook = stopAfterTwoMessages;
  bool waitingForReset = false;
  try { initializeStorage(); }
  catch (int) { waitingForReset = true; }
  testDelayHook = nullptr;
  assert(waitingForReset && EEPROM.beginCalls == 1 && testDelayMilliseconds == 2000);
  assert(Serial.messages.size() == 2);
  assert(Serial.messages[0] == "Settings storage initialization failed; reset the board to retry.");
  assert(Serial.messages[1] == Serial.messages[0]);
  assert(testFlushes == 0 && saveBegins == 0 && saveEnds == 0);
  initializeStorage(); // Simulated reset: success must not delay or log an error.
  assert(EEPROM.beginCalls == 2 && testDelayMilliseconds == 2000);
  assert(Serial.messages.size() == 2);
  assert(patternPeriodTicks(10000000, 1.0f, 3000) == 1667);
#endif
  loadConfig();
  assert(testFlushes == 1 && config.mode == POT_RPM && config.wheel < MAX_WHEELS);
  saveConfig();
  assert(testFlushes == 1);
  config.fixed_rpm = 4321;
  config.compressionDynamic = true;
  currentStatus.rpm = 3210;
  saveConfig();
  assert(testFlushes == 2 && saveBegins == 2 && saveEnds == 2);
  assert(testFlash[99] == 42);
  config = configTable{};
  currentStatus = {};
  loadConfig();
  assert(config.fixed_rpm == 4321 && config.compressionDynamic);
  assert(currentStatus.rpm == 3210 && currentStatus.base_rpm == 3210);

#if defined(ARDUSTIM_ESP32C3)
  EEPROM.failCommit = true;
  config.fixed_rpm = 4567;
  saveConfig();
  assert(testFlushes == 2);
  EEPROM.failCommit = false;
  saveConfig(); // Same values in RAM must still retry the failed commit.
  assert(testFlushes == 3);
  EEPROM.begin(32);
  loadConfig();
  assert(config.fixed_rpm == 4567);
  saveConfig();
  assert(testFlushes == 3);
#endif

  // Existing GUI binary packet remains exactly 18 bytes, little endian.
  configTable incoming = config;
  incoming.wheel = 6;
  incoming.mode = FIXED_RPM;
  incoming.fixed_rpm = 0x1234;
  edge_counter = 999;
  cycleDuration = 999;
  const uint8_t oldWheel = config.wheel;
  sendConfig(incoming);
  assert(config.fixed_rpm == 0x1234 && config.wheel == 6 && edge_counter == 0);
  assert(cycleDuration == 0 && timingUpdates == 1);
  assert(patternStops == 1 && patternStarts == 1 && wheelAtStop == oldWheel);
  Serial.receive({'C'});
  commandParser();
  assert(Serial.output.size() == 18);
  assert(Serial.output[3] == 0x34 && Serial.output[4] == 0x12);
  incoming.wheel = MAX_WHEELS;
  sendConfig(incoming);
  assert(config.wheel == 6 && timingUpdates == 1);
  incoming.wheel = 1;
  incoming.mode = MAX_MODES;
  sendConfig(incoming);
  assert(config.mode == FIXED_RPM && timingUpdates == 1);

  incoming = config;
  incoming.fixed_rpm = 5000;
  edge_counter = 77;
  cycleDuration = 88;
  sendConfig(incoming);
  assert(config.fixed_rpm == 5000 && edge_counter == 77 && cycleDuration == 88);
  assert(timingUpdates == 1); // An RPM edit must not restart the wheel phase.
  assert(patternStops == 1 && patternStarts == 1);

  Serial.receive({'S', 0});
  commandParser();
  assert(config.wheel == 0 && wheelAtStop == 6);
  assert(patternStops == 2 && patternStarts == 2 && edge_counter == 0);
  select_previous_wheel_cb();
  assert(config.wheel == MAX_WHEELS-1 && wheelAtStop == 0);
  Serial.receive({'X'});
  commandParser();
  assert(config.wheel == 0 && wheelAtStop == MAX_WHEELS-1);
  assert(patternStops == 4 && patternStarts == 4);
  Serial.receive({'S', MAX_WHEELS});
  commandParser();
  assert(patternStops == 4 && patternStarts == 4); // Invalid wheel cannot stop output.

  // Legacy AVR sweep packets carry each 16-bit value low byte first.
  Serial.receive({'r', 0x23, 0x01, 0x45, 0x23, 0x21, 0x03});
  commandParser();
  assert(config.sweep_low_rpm == 0x0123 && config.sweep_high_rpm == 0x2345);
  assert(config.sweep_interval == 0x0321 && config.mode == LINEAR_SWEPT_RPM);

  toggle_invert_primary_cb();
  assert(output_invert_mask == 1);
  toggle_invert_secondary_cb();
  assert(output_invert_mask == 3);
  toggle_invert_primary_cb();
  assert(output_invert_mask == 2);
  toggle_invert_secondary_cb();
  assert(output_invert_mask == 0);

  puts("Timing, GPIO mapping, serial protocol and buffered storage checks passed.");
}
