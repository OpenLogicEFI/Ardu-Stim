#ifndef ARDUSTIM_PATTERN_TIMING_H
#define ARDUSTIM_PATTERN_TIMING_H

#include <stdint.h>

// A scaler of 1 describes 120 samples per revolution: ticks = clock / (2 * RPM).
// TIM2 on the F401 has a 32-bit counter, so no changing prescaler is needed.
inline uint32_t patternPeriodTicks(uint32_t clockHz, float scaler, uint32_t rpm)
{
  if (rpm < 10) { rpm = 10; }
  const double ticks = clockHz / (2.0 * scaler * rpm);
  if (ticks >= UINT32_MAX) { return UINT32_MAX; }
  if (ticks < 1.0) { return 1; }
  return (uint32_t)(ticks + 0.5);
}

// All outputs are on the same GPIO bank and change with one atomic BSRR write.
inline uint32_t patternOutputBSRR(uint8_t state)
{
  const uint32_t pins = state & 0x07U;
  return pins | ((0x07U ^ pins) << 16);
}

#endif
