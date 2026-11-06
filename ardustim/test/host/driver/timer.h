#pragma once
#include <stdint.h>
#include <assert.h>
#define ESP_ERROR_CHECK(call) assert((call) == 0)
constexpr int TIMER_GROUP_0 = 0, TIMER_0 = 0, TIMER_COUNT_UP = 1;
constexpr int TIMER_PAUSE = 0, TIMER_ALARM_EN = 1, TIMER_AUTORELOAD_DIS = 0;
struct timer_config_t {
  int divider, counter_dir, counter_en, alarm_en, auto_reload;
};
static bool testRunning = false, testInterruptEnabled = false, testPending = false;
static bool testAlarmEnabled = false;
static uint64_t testCounter = 0, testAlarm = 0;
static bool (*testCallback)(void *) = nullptr;
inline int timer_init(int, int, const timer_config_t *) { return 0; }
inline int timer_enable_intr(int, int) { testInterruptEnabled = true; return 0; }
inline int timer_disable_intr(int, int) { testInterruptEnabled = false; return 0; }
inline int timer_pause(int, int) { testRunning = false; return 0; }
inline int timer_start(int, int) {
  assert(testInterruptEnabled && !testPending && testAlarmEnabled);
  testRunning = true;
  return 0;
}
inline int timer_isr_callback_add(int, int, bool (*cb)(void *), void *, int) {
  testCallback = cb;
  return 0;
}
inline int timer_set_counter_value(int, int, uint64_t value) { testCounter = value; return 0; }
inline int timer_set_alarm_value(int, int, uint64_t value) { testAlarm = value; return 0; }
inline int timer_set_alarm(int, int, int value) { testAlarmEnabled = value; return 0; }
inline uint64_t timer_group_get_counter_value_in_isr(int, int) { return testCounter; }
inline void timer_group_set_alarm_value_in_isr(int, int, uint64_t value) { testAlarm = value; }
inline void timer_group_enable_alarm_in_isr(int, int) { testAlarmEnabled = true; }
inline void timer_group_clr_intr_status_in_isr(int, int) { testPending = false; }
