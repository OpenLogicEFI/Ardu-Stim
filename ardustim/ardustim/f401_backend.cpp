#include "platform_compat.h"

#if defined(ARDUSTIM_F401)
#include <HardwareTimer.h>
#include "f401_backend.h"
#include "pattern_timing.h"

extern volatile uint16_t adc0;
extern volatile bool adc0_read_complete;
extern volatile uint16_t edge_counter;
extern volatile uint32_t cycleStartTime;
extern volatile uint32_t cycleDuration;

static HardwareTimer *patternTimer = nullptr;
static uint32_t timerClockHz;
static volatile uint32_t nextPeriodTicks;
static bool running = false;
static bool resumeAfterSave = false;

static void timerCallback()
{
  patternTick();
  // ARR is buffered: changing RPM cannot truncate the interval in progress.
  TIM2->ARR = nextPeriodTicks - 1;
}

void f401WriteOutputs(uint8_t state)
{
  GPIOB->BSRR = patternOutputBSRR(state);
}

void f401Setup()
{
  pinMode(PB0, OUTPUT); // crank
  pinMode(PB1, OUTPUT); // cam 1
  pinMode(PB2, OUTPUT); // cam 2
  f401WriteOutputs(0);
  pinMode(PA1, INPUT_ANALOG); // PA0 is connected to the Black Pill user button
  analogReadResolution(10); // Preserve the existing potentiometer RPM scaling

  static HardwareTimer timer(TIM2);
  patternTimer = &timer;
  patternTimer->pause();
  timerClockHz = patternTimer->getTimerClkFreq();
  patternTimer->setPrescaleFactor(1);
  patternTimer->setPreloadEnable(true);
  patternTimer->setInterruptPriority(0, 0); // Pulse edges take priority over USB (1)
  patternTimer->attachInterrupt(timerCallback);
}

void f401SetRPM(float scaler, uint32_t rpm)
{
  nextPeriodTicks = patternPeriodTicks(timerClockHz, scaler, rpm);
}

void f401Start()
{
  patternTimer->pause(); // Also suppress interrupts from the forced update below
  edge_counter = 0;
  cycleDuration = 0;
  cycleStartTime = micros();
  patternTimer->setOverflow(nextPeriodTicks, TICK_FORMAT);
  patternTimer->setCount(0);
  patternTimer->refresh();
  TIM2->SR = 0;
  NVIC_ClearPendingIRQ(TIM2_IRQn);
  patternTimer->resume();
  running = true;
}

void f401PollADC()
{
  static uint32_t lastRead = 0;
  const uint32_t now = micros();
  if ((uint32_t)(now - lastRead) >= 1000U)
  {
    lastRead = now;
    adc0 = analogRead(PA1);
    adc0_read_complete = true;
  }
}

void f401BeginSave()
{
  resumeAfterSave = running;
  if (running) { patternTimer->pause(); }
  running = false;
  // Flash erase stalls execution. Stop deliberately rather than emit a long tooth.
  if (patternTimer != nullptr) { f401WriteOutputs(0); }
}

void f401EndSave()
{
  if (resumeAfterSave) { f401Start(); }
}
#endif
