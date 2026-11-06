#pragma once
#include <stdint.h>
#include <assert.h>

struct TestTimerRegisters { uint32_t ARR = 0, SR = 0; };
static TestTimerRegisters testTIM2;
#define TIM2 (&testTIM2)
constexpr int TIM2_IRQn = 1, TICK_FORMAT = 0;
static bool testPendingIRQ = false;
inline void NVIC_ClearPendingIRQ(int) { testPendingIRQ = false; }

// VAL can roll over between the first read and the pending-bit check.
struct TestSysTickValue {
  uint32_t first = 0, second = 0;
  unsigned reads = 0;
  TestSysTickValue(uint32_t a = 83999, uint32_t b = 83999, unsigned n = 0)
    : first(a), second(b), reads(n) {}
  operator uint32_t() { return reads++ == 0 ? first : second; }
};
struct TestSysTick { TestSysTickValue VAL; uint32_t LOAD = 83999; };
struct TestSCB { uint32_t ICSR = 0; };
static TestSysTick testSysTick;
static TestSCB testSCB;
#define SysTick (&testSysTick)
#define SCB (&testSCB)
constexpr uint32_t SCB_ICSR_PENDSTSET_Msk = 1U << 26;
static uint32_t testPrimask = 0, testMilliseconds = 0;
inline uint32_t __get_PRIMASK() { return testPrimask; }
inline void __disable_irq() { testPrimask = 1; }
inline void __set_PRIMASK(uint32_t value) { testPrimask = value; }
inline uint32_t HAL_GetTick() { assert(testPrimask == 1); return testMilliseconds; }

class HardwareTimer {
public:
  static HardwareTimer *instance;
  bool running = false, preload = false;
  uint32_t activeARR = 0, count = 0;
  unsigned preemptPriority = 99;
  void (*callback)() = nullptr;
  explicit HardwareTimer(TestTimerRegisters *) { instance = this; }
  void pause() { running = false; }
  uint32_t getTimerClkFreq() { return 84000000; }
  void setPrescaleFactor(int value) { assert(value == 1); }
  void setPreloadEnable(bool value) { preload = value; }
  void setInterruptPriority(unsigned value, unsigned) { preemptPriority = value; }
  void attachInterrupt(void (*value)()) { callback = value; }
  void setOverflow(uint32_t ticks, int) { TIM2->ARR = ticks - 1; }
  void setCount(uint32_t value) { count = value; }
  void refresh() { activeARR = TIM2->ARR; TIM2->SR = 1; testPendingIRQ = true; }
  void resume() { running = true; }
};
struct TestGPIO { uint32_t BSRR = 0; };
static TestGPIO testGPIOB;
#define GPIOB (&testGPIOB)
