#include "platform_compat.h"
#if defined(ARDUSTIM_ESP32C3)
#include "pattern_backend.h"
#include "pattern_timing.h"
#include <driver/timer.h>
#include <soc/gpio_struct.h>

extern volatile uint16_t adc0, edge_counter;
extern volatile bool adc0_read_complete;
extern volatile uint32_t cycleStartTime, cycleDuration;
static volatile uint32_t nextPeriodTicks = 10000;
static uint64_t deadline;
static bool ready = false, running = false, resumeAfterSave = false;
// 80 MHz APB / 8 = 10 MHz: 100 ns timer resolution.
static constexpr uint32_t timerHz = 10000000;

static bool timerCallback(void *)
{
  patternTick();
  deadline += nextPeriodTicks;
  const uint64_t now = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, TIMER_0);
  // Do not emit a burst of catch-up samples after an overrun.
  if (deadline <= now) { deadline = now + nextPeriodTicks; }
  timer_group_set_alarm_value_in_isr(TIMER_GROUP_0, TIMER_0, deadline);
  timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
  return false;
}

void backendWriteOutputs(uint8_t state)
{
  // GPIO3/4/5 correspond to crank/cam1/cam2. Preserve all other pins.
  const uint32_t pins = (state & 7U) << 3;
  GPIO.out_w1tc.val = (7U << 3) ^ pins;
  GPIO.out_w1ts.val = pins;
}

void backendSetup()
{
  for (int pin = 3; pin <= 5; ++pin) { pinMode(pin, OUTPUT); }
  backendWriteOutputs(0);
  pinMode(0, INPUT);
  analogReadResolution(10);
  analogSetPinAttenuation(0, ADC_11db);
  timer_config_t cfg = {};
  cfg.divider = 8;
  cfg.counter_dir = TIMER_COUNT_UP;
  cfg.counter_en = TIMER_PAUSE;
  cfg.alarm_en = TIMER_ALARM_EN;
  cfg.auto_reload = TIMER_AUTORELOAD_DIS;
  ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &cfg));
  ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
  // Deliberately not IRAM-safe: wheel data lives in flash. All flash saves
  // stop this timer first. No radio stacks are started by this firmware.
  ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timerCallback, nullptr, 0));
  ready = true;
}

void backendSetRPM(float scaler, uint32_t rpm)
{
  nextPeriodTicks = patternPeriodTicks(timerHz, scaler, rpm);
}

void backendStart()
{
  ESP_ERROR_CHECK(timer_pause(TIMER_GROUP_0, TIMER_0));
  edge_counter = 0;
  cycleDuration = 0;
  cycleStartTime = micros();
  deadline = nextPeriodTicks;
  ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));
  ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, deadline));
  ESP_ERROR_CHECK(timer_set_alarm(TIMER_GROUP_0, TIMER_0, TIMER_ALARM_EN));
  ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));
  running = true;
}

void backendPollADC()
{
  static uint32_t lastRead = 0;
  const uint32_t now = micros();
  if ((uint32_t)(now - lastRead) >= 1000U)
  {
    lastRead = now;
    adc0 = analogRead(0);
    adc0_read_complete = true;
  }
}

void backendBeginSave()
{
  resumeAfterSave = running;
  if (ready)
  {
    ESP_ERROR_CHECK(timer_pause(TIMER_GROUP_0, TIMER_0));
    ESP_ERROR_CHECK(timer_disable_intr(TIMER_GROUP_0, TIMER_0));
    // Single-core C3: no timer callback can execute during the flash commit.
    backendWriteOutputs(0);
  }
  running = false;
}

void backendEndSave()
{
  if (resumeAfterSave)
  {
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    backendStart();
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
  }
}
#endif
