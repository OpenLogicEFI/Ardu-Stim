#pragma once
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <string>
#include <string.h>
using byte = uint8_t;
#define PROGMEM
#define highByte(v) ((uint8_t)((v) >> 8))
#define lowByte(v) ((uint8_t)(v))
#define constrain(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
inline uint16_t word(uint8_t high, uint8_t low) { return (uint16_t(high) << 8) | low; }
inline void noInterrupts() {}
inline void interrupts() {}
inline uint32_t micros() { return 1234; }
extern unsigned long testDelayMilliseconds;
extern void (*testDelayHook)();
inline void delay(unsigned long milliseconds) {
  testDelayMilliseconds += milliseconds;
  if (testDelayHook) { testDelayHook(); }
}
struct TestSerial {
  std::vector<uint8_t> input, output;
  std::vector<std::string> messages;
  size_t offset = 0;
  void begin(unsigned long) {}
  int available() { return int(input.size() - offset); }
  int read() { return offset < input.size() ? input[offset++] : -1; }
  void write(uint8_t v) { output.push_back(v); }
  template<typename T> void print(T) {}
  void println(const char *message) { messages.push_back(message); }
  template<typename T> void println(T) {}
  void receive(std::vector<uint8_t> bytes) { input = bytes; offset = 0; output.clear(); }
};
extern TestSerial Serial;

#if defined(ARDUSTIM_BACKEND_TEST)
constexpr int OUTPUT = 1, INPUT = 0, INPUT_ANALOG = 2, ADC_11db = 3;
constexpr int PB0 = 0, PB1 = 1, PB2 = 2, PA1 = 3;
inline void pinMode(int, int) {}
inline void analogReadResolution(int) {}
inline void analogSetPinAttenuation(int, int) {}
inline uint16_t analogRead(int) { return 123; }
#endif
