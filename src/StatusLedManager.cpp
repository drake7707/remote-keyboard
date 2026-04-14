#include "StatusLedManager.h"

void StatusLedManager::begin(int pin)
{
  _pin = pin;
  _ledState = 0;
  _ledStateTime = millis_now();
  _status = APP_BT_DISCONNECTED;

  gpio_config_t gpioConfig = {};
  gpioConfig.pin_bit_mask = 1ULL << pin;
  gpioConfig.mode = GPIO_MODE_OUTPUT;
  gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
  gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpioConfig.intr_type = GPIO_INTR_DISABLE;
  gpio_config(&gpioConfig);
  gpio_set_level((gpio_num_t)pin, 0);
}

void StatusLedManager::flashLed(int times, uint32_t onDurationMs, uint32_t offDurationMs)
{
  _flashActive = true;
  _flashRemaining = times;
  _flashOnTime = onDurationMs;
  _flashOffTime = offDurationMs;
  _flashLedOn = true;
  gpio_set_level((gpio_num_t)_pin, 1);
  _flashStateTime = millis_now();
}

void StatusLedManager::flashButtonPressed(uint32_t buttonIndex)
{
  flashLed(1, 150, 0);
}

void StatusLedManager::flashTargetChanged(uint32_t targetIndex)
{
  if (targetIndex == -1)
    flashLed(1, 1000, 100);
  else
    flashLed(targetIndex+1, 150, 100);
}

void  StatusLedManager::flashConfigModeEntry() {
  flashLed(3, 100, 100);
}

void StatusLedManager::flashKeymapChanged(uint32_t keymapIndex)
{
  flashLed(keymapIndex, 150, 100);
}

void StatusLedManager::resetLedState()
{
  _ledState = 0;
  _ledStateTime = millis_now();
  gpio_set_level((gpio_num_t)_pin, 0);
}

void StatusLedManager::update()
{
  uint32_t now = millis_now();

  // A pending flash animation takes priority over the status blink pattern
  if (_flashActive)
  {
    if (_flashLedOn)
    {
      if (now - _flashStateTime >= _flashOnTime)
      {
        _flashRemaining--;
        if (_flashRemaining == 0)
        {
          // Last flash complete — turn off and return to status blink
          gpio_set_level((gpio_num_t)_pin, 0);
          _flashActive = false;
          _ledStateTime = now; // reset status blink timer
        }
        else
        {
          // More flashes to go — turn off and wait for off delay
          gpio_set_level((gpio_num_t)_pin, 0);
          _flashLedOn = false;
          _flashStateTime = now;
        }
      }
    }
    else
    {
      if (now - _flashStateTime >= _flashOffTime)
      {
        // Start next flash
        gpio_set_level((gpio_num_t)_pin, 1);
        _flashLedOn = true;
        _flashStateTime = now;
      }
    }
    return;
  }

  if (_status == APP_BT_DISCONNECTED || _status == APP_CONFIG || _status == APP_BT_CONNECTED_ADVERTISING)
  {
    if ((now - _ledStateTime) > (uint32_t)_ledDelays[_status][_ledState])
    {
      _ledState = 1 - _ledState;
      gpio_set_level((gpio_num_t)_pin, _ledState);
      _ledStateTime = now;
    }
  }
}
