#pragma once

#include <Arduino.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "HardwareConfig.h"
#include "StatusLedManager.h"
#include "ButtonManager.h"

// ---------------------------------------------------------------------------
// SleepManager — reduces power consumption during normal operation.
//
// Enters ESP32-C3 light sleep between main-loop iterations.  The BLE
// controller continues to operate independently while the CPU is sleeping.
//
// Two wakeup sources are armed before each sleep:
//   • GPIO  — when all buttons are idle the column pins are driven LOW so
//             any keypress pulls a row pin LOW and wakes the CPU immediately.
//             The Keypad library reconfigures the pins on the next getKey().
//   • Timer — capped at min(msUntilNextLedUpdate, MAX_SLEEP_MS) so that the
//             LED blink pattern and BLE state checks are serviced on time
//             even when no key is pressed.
//
// Do NOT call sleep() during config mode; the WiFi AP needs the CPU to
// remain continuously responsive (use delay(10) instead).
// ---------------------------------------------------------------------------
class SleepManager {
public:
  SleepManager(StatusLedManager& led, ButtonManager& buttons)
    : _led(led), _buttons(buttons) {}

  // Enter light sleep for up to the time remaining until the next LED update.
  // Returns immediately (with a 1 ms yield) when a transition is already due.
  void sleep() {
    unsigned long ledMs = _led.msUntilNextUpdate();
    if (ledMs == 0) {
      // LED transition is overdue — yield briefly so the caller's loop
      // can invoke ledManager.update() without busy-spinning at full speed.
      delay(1);
      return;
    }

    // Cap sleep time so BLE/system events are still serviced regularly.
    const unsigned long MAX_SLEEP_MS = 50UL;
    unsigned long sleepMs = (ledMs > MAX_SLEEP_MS) ? MAX_SLEEP_MS : ledMs;

    // When no button is currently held, drive all column pins LOW and enable
    // GPIO-level wakeup on the row pins.  Any button press will then pull a
    // row pin LOW and wake the CPU immediately.
    bool gpioWakeEnabled = _buttons.isIdle();
    if (gpioWakeEnabled) {
      for (int i = 0; i < 3; i++) {
        gpio_set_direction((gpio_num_t)KEYPAD_COL_PINS[i], GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)KEYPAD_COL_PINS[i], 0);
      }
      for (int i = 0; i < 3; i++) {
        gpio_wakeup_enable((gpio_num_t)KEYPAD_ROW_PINS[i], GPIO_INTR_LOW_LEVEL);
      }
      esp_sleep_enable_gpio_wakeup();
    }

    esp_sleep_enable_timer_wakeup((uint64_t)sleepMs * 1000ULL);
    esp_light_sleep_start();

    // Disable wakeup sources so they don't interfere with the next sleep cycle.
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    if (gpioWakeEnabled) {
      for (int i = 0; i < 3; i++) {
        gpio_wakeup_disable((gpio_num_t)KEYPAD_ROW_PINS[i]);
      }
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
    }
  }

private:
  StatusLedManager& _led;
  ButtonManager&    _buttons;
};
