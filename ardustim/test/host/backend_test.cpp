#include <assert.h>
#include <cstdio>
#include "pattern_backend.h"

// Compile the production backends against peripheral mocks, including callbacks.
#if defined(ARDUSTIM_F401)
#include "f401_backend.cpp"
HardwareTimer *HardwareTimer::instance = nullptr;
#else
#include "esp32c3_backend.cpp"
#endif

volatile uint16_t adc0 = 0, edge_counter = 0;
volatile bool adc0_read_complete = false;
volatile uint32_t cycleStartTime = 0, cycleDuration = 0;
void patternTick() { ++edge_counter; }

int main()
{
  backendSetup();
  backendSetRPM(0.03333f, 10); // Sparse wheel, long period.
  backendStart();
  edge_counter = 3;
  cycleDuration = 12345;
#if defined(ARDUSTIM_F401)
  HardwareTimer *timer = HardwareTimer::instance;
  timer->count = 123;
  testPendingIRQ = true;
  TIM2->SR = 1;
#else
  testCounter = 123;
  testPending = true;
#endif
  backendStop();
  backendSetRPM(3.0f, 9000); // Dense wheel, short period.
  backendStart();
  assert(edge_counter == 0 && cycleDuration == 0);
#if defined(ARDUSTIM_F401)
  assert(timer->running && timer->count == 0 && !testPendingIRQ && TIM2->SR == 0);
  assert(timer->preload && timer->activeARR == 1555);
  timer->callback();
  assert(edge_counter == 1 && TIM2->ARR == 1555);

  // Ordinary RPM updates remain buffered and preserve phase.
  backendSetRPM(1.0f, 3000);
  assert(timer->activeARR == 1555 && edge_counter == 1);
  timer->callback();
  assert(timer->activeARR == 1555 && TIM2->ARR == 13999);

  // A pending millisecond is counted once without clearing it or changing PRIMASK.
  testMilliseconds = 42;
  testSysTick.VAL = {42000, 42000, 0};
  testSCB.ICSR = 0;
  assert(backendMicros() == 42500 && testPrimask == 0);
  testSysTick.VAL = {1, 83916, 0};
  testSCB.ICSR = SCB_ICSR_PENDSTSET_Msk;
  assert(backendMicros() == 43001 && testPrimask == 0);
  assert(testSCB.ICSR == SCB_ICSR_PENDSTSET_Msk);
  testSysTick.VAL = {0, 83916, 0};
  assert(backendMicros() == 43001 && testPrimask == 0); // Exact reload boundary.
  testPrimask = 1;
  testSysTick.VAL = {83916, 83916, 0};
  assert(backendMicros() == 43001 && testPrimask == 1);
  testSCB.ICSR = 0;
  testMilliseconds = 43;
  testSysTick.VAL = {83916, 83916, 0};
  assert(backendMicros() == 43001 && testPrimask == 1);
  testPrimask = 0;
  testMilliseconds = UINT32_MAX;
  testSCB.ICSR = SCB_ICSR_PENDSTSET_Msk;
  testSysTick.VAL = {83916, 83916, 0};
  assert(backendMicros() == 1); // HAL millisecond wrap.
#else
  assert(testRunning && testInterruptEnabled && !testPending);
  assert(testCounter == 0 && testAlarm == 185);
  testCounter = testAlarm;
  testCallback(nullptr);
  assert(edge_counter == 1 && testAlarm == 370);
  backendSetRPM(1.0f, 3000);
  assert(testAlarm == 370 && edge_counter == 1); // No phase reset on RPM edit.
#endif

  backendBeginSave();
  backendEndSave();
  assert(edge_counter == 0 && cycleDuration == 0);
#if defined(ARDUSTIM_F401)
  assert(timer->running && timer->activeARR == 13999 && !testPendingIRQ);
#else
  assert(testRunning && testInterruptEnabled && !testPending && testAlarm == 1667);
#endif
  puts("Backend restart, pending event and timestamp checks passed.");
}
