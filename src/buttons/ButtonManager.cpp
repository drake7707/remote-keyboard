#include "ButtonManager.h"

ButtonManager::ButtonManager()
  : _keypad(makeKeymap(_buttons), _rowPins, _colPins, ROWS, COLS)
{}


void ButtonManager::begin() {
  for (int i = 0; i < MAX_BTNS; i++) {
    _repeating[i]     = false;
    _longPressTime[i] = SHORT_PRESS_MAX;
    _active[i]        = false;
    _longFired[i]     = false;
    _comboFired[i]    = false;
    _pressStart[i]    = 0;
    _lastRepeat[i]    = 0;
  }
  _keypad.setHoldTime(SHORT_PRESS_MAX);
}

void ButtonManager::setShortPressHandler(void (*handler)(char btn))           { _shortPressHandler = handler; }
void ButtonManager::setLongPressHandler(void (*handler)(char btn))            { _longPressHandler  = handler; }
void ButtonManager::setComboHandler(void (*handler)(char held, char pressed)) { _comboHandler      = handler; }

void ButtonManager::setButtonRepeating(char btn, bool repeating) {
  int buttonIndex = _btnIdx(btn);
  if (buttonIndex >= 0) _repeating[buttonIndex] = repeating;
}

void ButtonManager::setButtonLongPressTime(char btn, uint32_t ms) {
  int buttonIndex = _btnIdx(btn);
  if (buttonIndex >= 0) _longPressTime[buttonIndex] = ms;
}

void ButtonManager::setPinConfiguration(const uint8_t* rowPins, const uint8_t* colPins) {
  memcpy(_rowPins, rowPins, sizeof(_rowPins));
  memcpy(_colPins, colPins, sizeof(_colPins));
}

void ButtonManager::print_keypad_state() {
  char stateStr[64] = {};
  size_t writePos = 0;
  for (int i = 0; i < LIST_MAX; i++) {
    char     btn = _keypad.key[i].kchar;
    KeyState keyState  = _keypad.key[i].kstate;
    if (btn == NO_KEY) continue;
    int buttonIndex = _btnIdx(btn);
    if (buttonIndex < 0) continue;
    if (writePos < sizeof(stateStr) - 5) {
      char st = keyState == PRESSED ? 'P' : keyState == HOLD ? 'H' : keyState == RELEASED ? 'R' : 'I';
      writePos += snprintf(stateStr + writePos, sizeof(stateStr) - writePos, "%c=%c ", btn, st);
    }
  }
  if (strcmp(stateStr, _oldPrint) != 0) {
    printf("[BUTTON] BUTTON STATE: %s\n", stateStr);
    strncpy(_oldPrint, stateStr, sizeof(_oldPrint) - 1);
  }
}

void ButtonManager::update() {
  _keypad.getKeys();
  uint32_t now = (uint32_t)(esp_timer_get_time() / 1000LL);

  if (DEBUG) print_keypad_state();

  for (int i = 0; i < LIST_MAX; i++) {
    char     btn         = _keypad.key[i].kchar;
    KeyState keyState    = _keypad.key[i].kstate;

    if (btn == NO_KEY) continue;

    int buttonIndex = _btnIdx(btn);
    if (buttonIndex < 0) continue;

    switch (keyState) {

      case PRESSED:
        if (!_active[buttonIndex]) {
          if (DEBUG) printf("[BUTTON] %c PRESSED\n", btn);
          _active[buttonIndex]     = true;
          _pressStart[buttonIndex] = now;
          _lastRepeat[buttonIndex] = now;
          _longFired[buttonIndex]  = false;
          _comboFired[buttonIndex] = false;

          // Combo detection: another button currently active?
          // No timing guard -- both buttons can appear as PRESSED in the same
          // scan cycle (now - _pressStart[j] == 0), so a threshold would
          // silently discard the combo.  Whichever button was registered first
          // in the key[] array is treated as the "held" button.
          // Both buttons are marked combo-involved to suppress their individual
          // short-press events on release.
          for (int j = 0; j < MAX_BTNS; j++) {
            if (j == buttonIndex || !_active[j]) continue;
            if (_comboHandler) _comboHandler(_btnChar(j), btn);
            _comboFired[buttonIndex] = true;
            _comboFired[j]           = true;
            break;
          }

          if (!_comboFired[buttonIndex] && _repeating[buttonIndex]) {
            if (_shortPressHandler) _shortPressHandler(btn);
          }
        }
        break;

      case HOLD:
        if (_active[buttonIndex]) {
          uint32_t heldMs = now - _pressStart[buttonIndex];
          if (_repeating[buttonIndex]) {
            if (heldMs >= SHORT_PRESS_MAX && (now - _lastRepeat[buttonIndex]) >= REPEAT_MS) {
              if (_shortPressHandler) _shortPressHandler(btn);
              _lastRepeat[buttonIndex] = now;
            }
          } else {
            if (!_longFired[buttonIndex] && !_comboFired[buttonIndex] && heldMs >= _longPressTime[buttonIndex]) {
              if (_longPressHandler) _longPressHandler(btn);
              _longFired[buttonIndex] = true;
            }
          }
        }
        break;

      case RELEASED:
        if (_active[buttonIndex]) {
          if (DEBUG) printf("[BUTTON] %c RELEASED\n", btn);
          uint32_t heldMs = now - _pressStart[buttonIndex];
          _active[buttonIndex] = false;
          // Short press fires on release for a genuine tap (< SHORT_PRESS_MAX,
          // not consumed by a long-press or combo)
          if (!_longFired[buttonIndex] && !_comboFired[buttonIndex] && !_repeating[buttonIndex] &&
              heldMs < SHORT_PRESS_MAX) {
            if (_shortPressHandler) _shortPressHandler(btn);
          }
        }
        break;

      case IDLE:
        if (_active[buttonIndex]) {
          // RELEASED state was missed -- treat as release now
          uint32_t heldMs = now - _pressStart[buttonIndex];
          _active[buttonIndex] = false;
          if (!_longFired[buttonIndex] && !_comboFired[buttonIndex] && !_repeating[buttonIndex] &&
              heldMs < SHORT_PRESS_MAX) {
            if (_shortPressHandler) _shortPressHandler(btn);
          }
        }
        break;
    }
  }
}

bool ButtonManager::isIdle() const {
  for (int i = 0; i < MAX_BTNS; i++) if (_active[i]) return false;
  return true;
}

void ButtonManager::drainButton(int timeoutMs) {
  uint32_t start = (uint32_t)(esp_timer_get_time() / 1000LL);
  while ((uint32_t)(esp_timer_get_time() / 1000LL) - start < (uint32_t)timeoutMs) {
    _keypad.getKeys();
    bool anyActive = false;
    for (int i = 0; i < LIST_MAX; i++) {
      KeyState keyState = _keypad.key[i].kstate;
      if (keyState == PRESSED || keyState == HOLD || keyState == RELEASED) { anyActive = true; break; }
    }
    if (!anyActive) break;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  for (int i = 0; i < MAX_BTNS; i++) _active[i] = false;
}

int  ButtonManager::_btnIdx(char btn) { return (btn >= '1' && btn <= '9') ? btn - '1' : -1; }
char ButtonManager::_btnChar(int idx) { return '1' + idx; }
