#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// AppStatus — shared application-state enum used by multiple managers
// ---------------------------------------------------------------------------
enum AppStatus {
  APP_BT_DISCONNECTED = 0,  // Not connected to BT
  APP_CONFIG          = 1,  // Config mode (AP active, rapid blink)
  APP_CONNECTED       = 2,  // BT connected, main menu
  APP_CONNECTED_BLINK = 3   // BT connected, keymap indicator flash
};

// ---------------------------------------------------------------------------
// StatusLedManager — manages the status LED blink patterns and app state
// ---------------------------------------------------------------------------
class StatusLedManager {
public:
  void begin(int pin) {
    _pin          = pin;
    _ledState     = 0;
    _ledStateTime = millis();
    _status       = APP_BT_DISCONNECTED;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
  }

  // Flash the LED n times (blocking)
  void flashLed(int times, int length, int delayTime) {
    for (int i = 0; i < times; i++) {
      digitalWrite(_pin, HIGH);
      delay(length);
      digitalWrite(_pin, LOW);
      if (i < (times - 1)) delay(delayTime);
    }
  }

  void setStatus(AppStatus s) { _status = s; }
  AppStatus getStatus()       { return _status; }

  // Reset LED to off and restart the blink timer
  void resetLedState() {
    _ledState     = 0;
    _ledStateTime = millis();
    digitalWrite(_pin, LOW);
  }

  // Drive blink pattern — call every loop iteration when keypad is idle
  void update() {
    if (_status == APP_CONNECTED_BLINK) {
      if ((millis() - _ledStateTime) > (unsigned long)_keymapIndicatorLedDelays[_ledState]) {
        _ledState = 1 - _ledState;
        digitalWrite(_pin, _ledState);
        _ledStateTime = millis();
        if (_ledState == 0) {
          if (--_keymapIndicatorCountdown == 0) _status = APP_CONNECTED;
        }
      }
    } else if (_status != APP_CONNECTED) {
      // APP_BT_DISCONNECTED / APP_CONFIG: run the blink timer
      if ((millis() - _ledStateTime) > (unsigned long)_ledDelays[_status][_ledState]) {
        _ledState = 1 - _ledState;
        digitalWrite(_pin, _ledState);
        _ledStateTime = millis();
      }
    }
    // APP_CONNECTED: LED stays off; key-press events use flashLed() directly
  }

private:
  int       _pin                      = 0;
  AppStatus _status                   = APP_BT_DISCONNECTED;
  int       _ledState                 = 0;
  unsigned long _ledStateTime         = 0;
  int       _keymapIndicatorCountdown = 0;

  int _ledDelays[4][2] = {
    {  500,  500 },  // APP_BT_DISCONNECTED
    {  100, 3000 },  // APP_CONFIG
    { 3000,  100 },  // APP_CONNECTED
    { 3000,  100 }   // APP_CONNECTED_BLINK
  };
  int _keymapIndicatorLedDelays[2] = {100, 50};
};
