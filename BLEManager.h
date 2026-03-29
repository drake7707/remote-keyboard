#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#include "HIDTypes.h"
#include "KeyCodes.h"

extern const int DEBUG;

// ---------------------------------------------------------------------------
// Combined HID report descriptor — keyboard (Report ID 1) + mouse (Report ID 2)
// ---------------------------------------------------------------------------
static const uint8_t _hidReportDesc[] = {
  // --- Keyboard (Report ID 1) ---
  0x05, 0x01,   // Usage Page (Generic Desktop)
  0x09, 0x06,   // Usage (Keyboard)
  0xA1, 0x01,   // Collection (Application)
  0x85, 0x01,   //   Report ID (1)
  0x05, 0x07,   //   Usage Page (Key Codes)
  0x19, 0xE0,   //   Usage Minimum (Left Control)
  0x29, 0xE7,   //   Usage Maximum (Right GUI)
  0x15, 0x00,   //   Logical Minimum (0)
  0x25, 0x01,   //   Logical Maximum (1)
  0x75, 0x01,   //   Report Size (1)
  0x95, 0x08,   //   Report Count (8)
  0x81, 0x02,   //   Input (Data, Var, Abs)    — modifier byte
  0x95, 0x01,   //   Report Count (1)
  0x75, 0x08,   //   Report Size (8)
  0x81, 0x01,   //   Input (Const)             — reserved byte
  0x95, 0x06,   //   Report Count (6)
  0x75, 0x08,   //   Report Size (8)
  0x25, 0x65,   //   Logical Maximum (101)
  0x05, 0x07,   //   Usage Page (Key Codes)
  0x19, 0x00,   //   Usage Minimum (0)
  0x29, 0x65,   //   Usage Maximum (101)
  0x81, 0x00,   //   Input (Data, Array, Abs)  — key slots
  0xC0,         // End Collection (keyboard)

  // --- Mouse (Report ID 2) ---
  0x05, 0x01,   // Usage Page (Generic Desktop)
  0x09, 0x02,   // Usage (Mouse)
  0xA1, 0x01,   // Collection (Application)
  0x85, 0x02,   //   Report ID (2)
  0x09, 0x01,   //   Usage (Pointer)
  0xA1, 0x00,   //   Collection (Physical)
  0x05, 0x09,   //     Usage Page (Buttons)
  0x19, 0x01,   //     Usage Minimum (Button 1)
  0x29, 0x03,   //     Usage Maximum (Button 3)
  0x15, 0x00,   //     Logical Minimum (0)
  0x25, 0x01,   //     Logical Maximum (1)
  0x75, 0x01,   //     Report Size (1)
  0x95, 0x03,   //     Report Count (3)
  0x81, 0x02,   //     Input (Data, Var, Abs)  — 3 button bits
  0x75, 0x05,   //     Report Size (5)
  0x95, 0x01,   //     Report Count (1)
  0x81, 0x01,   //     Input (Const)           — 5 padding bits
  0x05, 0x01,   //     Usage Page (Generic Desktop)
  0x09, 0x30,   //     Usage (X)
  0x09, 0x31,   //     Usage (Y)
  0x09, 0x38,   //     Usage (Wheel)
  0x15, 0x81,   //     Logical Minimum (-127)
  0x25, 0x7F,   //     Logical Maximum (127)
  0x75, 0x08,   //     Report Size (8)
  0x95, 0x03,   //     Report Count (3)
  0x81, 0x06,   //     Input (Data, Var, Rel)  — X, Y, Wheel
  0xC0,         //   End Collection (Physical)
  0xC0,         // End Collection (mouse Application)

  // --- Touch Screen (Report ID 3) — 2-contact digitizer for pinch-to-zoom ---
  //
  // Each contact (5 bytes):
  //   Byte 0 : [tip(1 bit) | contact_id(4 bits) | padding(3 bits)]
  //   Bytes 1-2 : X  (uint16 LE, 0–4096)
  //   Bytes 3-4 : Y  (uint16 LE, 0–4096)
  // Byte 10  : contact count (0–2)
  // Total payload: 11 bytes
  //
  0x05, 0x0D,        // Usage Page (Digitizers)
  0x09, 0x04,        // Usage (Touch Screen)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x03,        //   Report ID (3)

  // Contact 1
  0x09, 0x22,        //   Usage (Finger)
  0xA1, 0x02,        //   Collection (Logical)
  0x09, 0x42,        //     Usage (Tip Switch)
  0x15, 0x00,        //     Logical Minimum (0)
  0x25, 0x01,        //     Logical Maximum (1)
  0x75, 0x01,        //     Report Size (1)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 1 bit: tip switch
  0x09, 0x51,        //     Usage (Contact Identifier)
  0x25, 0x0F,        //     Logical Maximum (15)
  0x75, 0x04,        //     Report Size (4)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 4 bits: contact id
  0x75, 0x03,        //     Report Size (3)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x01,        //     Input (Const)          — 3 bits: padding → byte boundary
  0x05, 0x01,        //     Usage Page (Generic Desktop)
  0x09, 0x30,        //     Usage (X)
  0x15, 0x00,        //     Logical Minimum (0)
  0x26, 0x00, 0x10,  //     Logical Maximum (4096)
  0x75, 0x10,        //     Report Size (16)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 16 bits: X
  0x09, 0x31,        //     Usage (Y)
  0x26, 0x00, 0x10,  //     Logical Maximum (4096)
  0x75, 0x10,        //     Report Size (16)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 16 bits: Y
  0xC0,              //   End Collection (Contact 1)

  // Contact 2
  0x05, 0x0D,        //   Usage Page (Digitizers)
  0x09, 0x22,        //   Usage (Finger)
  0xA1, 0x02,        //   Collection (Logical)
  0x09, 0x42,        //     Usage (Tip Switch)
  0x15, 0x00,        //     Logical Minimum (0)
  0x25, 0x01,        //     Logical Maximum (1)
  0x75, 0x01,        //     Report Size (1)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 1 bit: tip switch
  0x09, 0x51,        //     Usage (Contact Identifier)
  0x25, 0x0F,        //     Logical Maximum (15)
  0x75, 0x04,        //     Report Size (4)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 4 bits: contact id
  0x75, 0x03,        //     Report Size (3)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x01,        //     Input (Const)          — 3 bits: padding → byte boundary
  0x05, 0x01,        //     Usage Page (Generic Desktop)
  0x09, 0x30,        //     Usage (X)
  0x15, 0x00,        //     Logical Minimum (0)
  0x26, 0x00, 0x10,  //     Logical Maximum (4096)
  0x75, 0x10,        //     Report Size (16)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 16 bits: X
  0x09, 0x31,        //     Usage (Y)
  0x26, 0x00, 0x10,  //     Logical Maximum (4096)
  0x75, 0x10,        //     Report Size (16)
  0x95, 0x01,        //     Report Count (1)
  0x81, 0x02,        //     Input (Data, Var, Abs) — 16 bits: Y
  0xC0,              //   End Collection (Contact 2)

  // Contact Count
  0x05, 0x0D,        //   Usage Page (Digitizers)
  0x09, 0x54,        //   Usage (Contact Count)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x02,        //   Logical Maximum (2)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x02,        //   Input (Data, Var, Abs) — 8 bits: contact count
  0xC0               // End Collection (touch Application)
};

// ASCII 32..126 → HID scan code. Bit 7 set means LEFT_SHIFT is also needed.
// US QWERTY layout.
static const uint8_t _asciiToHid[95] PROGMEM = {
  0x2C,                                                       // ' '  32
  0x9E,0xB4,0xA0,0xA1,0xA2,0xA4,0x34,0xA6,0xA7,0xA5,        // !-*  33-42
  0xAE,0x36,0x2D,0x37,0x38,                                  // +-/  43-47
  0x27,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,        // 0-9  48-57
  0xB3,0x33,0xB6,0x2E,0xB7,0xB8,0x9F,                       // :-@  58-64
  0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,                  // A-H  65-72
  0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,                  // I-P  73-80
  0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,        // Q-Z  81-90
  0x2F,0x31,0x30,0xA3,0xAD,0x35,                            // [-`  91-96
  0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,                  // a-h  97-104
  0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,                  // i-p  105-112
  0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,        // q-z  113-122
  0xAF,0xB1,0xB0,0xB5                                        // {-~  123-126
};

struct KbReport    { uint8_t mod; uint8_t reserved; uint8_t keys[6]; };
struct MouseReport { uint8_t buttons; int8_t x; int8_t y; int8_t scroll; };

// Per-contact layout for Report ID 3 (touch screen, 5 bytes each):
//   Byte 0  : bit 0 = Tip Switch, bits 1-4 = Contact ID, bits 5-7 = padding
//   Bytes 1-2 : X  (uint16 LE, 0–4096)
//   Bytes 3-4 : Y  (uint16 LE, 0–4096)
struct TouchContact { uint8_t flags; uint16_t x; uint16_t y; } __attribute__((packed));
// Full Report ID 3 payload (11 bytes, Report ID prefix handled by NimBLE):
struct TouchReport  { TouchContact c1; TouchContact c2; uint8_t contactCount; } __attribute__((packed));

// ---------------------------------------------------------------------------
// BLEManager — manages all BLE keyboard functionality
// NimBLE persists CCCD (notification subscription) in NVS per bonded peer,
// so keystrokes continue working after device resets without re-pairing.
// Security: bonding with Secure Connections, Just Works (no MITM, no pin).
// ---------------------------------------------------------------------------
class BLEManager : public NimBLEServerCallbacks {
public:
  BLEManager(const char* mfr, uint8_t bat) : _mfr(mfr), _bat(bat) {}

  void begin(const char* name) {
    memset(&_rep, 0, sizeof(_rep));
    NimBLEDevice::init(name);
    // init() must come first. setSecurityAuth() after init() sets bond=true,
    // MITM=true, SC=true. The IO capability is left at NimBLE's default —
    // overriding it with NO_INPUT_OUTPUT prevents Android from bonding.
    NimBLEDevice::setSecurityAuth(true, true, true);
    if (DEBUG) Serial.printf("Bonds in NVS: %d\n", NimBLEDevice::getNumBonds());

    _srv = NimBLEDevice::createServer();
    _srv->setCallbacks(this);
    _srv->advertiseOnDisconnect(true); // NimBLE restarts advertising automatically

    _hid   = new NimBLEHIDDevice(_srv);
    _input      = _hid->getInputReport(1); // keyboard
    _mouseInput = _hid->getInputReport(2); // mouse
    _touchInput = _hid->getInputReport(3); // touch screen (pinch zoom)

    _hid->setManufacturer(_mfr);
    _hid->setPnp(0x02, 0xe502, 0xa111, 0x0210);
    _hid->setHidInfo(0x00, 0x02);
    _hid->setReportMap((uint8_t*)_hidReportDesc, sizeof(_hidReportDesc));
    _hid->startServices();
    _hid->setBatteryLevel(_bat);

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->setAppearance(HID_KEYBOARD);
    adv->addServiceUUID(_hid->getHidService()->getUUID());

    // Include device name in scan response so Android/Windows show it in
    // the pairing list and recognise it as a keyboard.
    NimBLEAdvertisementData scanData;
    scanData.setName(name);
    adv->setScanResponseData(scanData);

    // If a bonded peer exists, use directed advertising so Android/Windows
    // auto-reconnects without user interaction after a device reset.
    // After the directed window expires NimBLE falls back to undirected advertising.
    if (NimBLEDevice::getNumBonds() > 0) {
      NimBLEAddress peer = NimBLEDevice::getBondedAddress(0);
      if (DEBUG) {
        Serial.print("Directed adv to bonded peer: ");
        Serial.println(peer.toString().c_str());
      }
      adv->start(0, &peer);
    } else {
      adv->start();
    }
  }

  void end() {
    NimBLEDevice::deinit(false); // false = keep bond data in NVS
    _connected  = false;
    _srv        = nullptr;
    _hid        = nullptr;
    _input      = nullptr;
    _mouseInput = nullptr;
    _touchInput = nullptr;
  }

  bool isConnected() { return _connected; }

  // Mouse / touch action codes (MOUSE_UP … MOUSE_PINCH_ZOOM_OUT) are routed to
  // the HID mouse (Report ID 2) or touch (Report ID 3) reports instead of the
  // keyboard report so that apps that do not respond to keyboard input (e.g.
  // Waze) still receive usable pointer / scroll / pinch events.
  // The codes are defined contiguously (0xE0–0xE8) in KeyCodes.h; any new
  // mouse/touch code added there must also be handled in mouseAction() below.
  void write(uint8_t key) {
    if (key >= MOUSE_UP && key <= MOUSE_PINCH_ZOOM_OUT) {
      mouseAction(key);
      return;
    }
    if (DEBUG) Serial.printf("[BLE] write: key=0x%02X (%d)\n", key, key);
    press(key); delay(10); releaseAll();
  }

  // Hold key (accumulates modifiers / keys until releaseAll)
  void press(uint8_t key) {
    uint8_t scan = 0, modBit = 0;
    toHID(key, scan, modBit);
    if (DEBUG) Serial.printf("[BLE] press: key=0x%02X -> scan=0x%02X mod=0x%02X\n", key, scan, modBit);
    if (modBit)    _rep.mod |= modBit;
    if (scan)      for (int i = 0; i < 6; i++) if (!_rep.keys[i]) { _rep.keys[i] = scan; break; }
    send();
  }

  void releaseAll() { memset(&_rep, 0, sizeof(_rep)); send(); }

  // Delete all stored BLE bonds from NVS
  static void clearAllBonds() { NimBLEDevice::deleteAllBonds(); }

private:
  const char*           _mfr;
  uint8_t               _bat;
  bool                  _connected  = false;
  NimBLEServer*         _srv        = nullptr;
  NimBLEHIDDevice*      _hid        = nullptr;
  NimBLECharacteristic* _input      = nullptr; // keyboard Report ID 1
  NimBLECharacteristic* _mouseInput = nullptr; // mouse    Report ID 2
  NimBLECharacteristic* _touchInput = nullptr; // touch    Report ID 3
  KbReport              _rep        = {};
  MouseReport           _mouseRep   = {};

  // Pixels moved per pan event (used in repeating mode at ~100 ms intervals).
  // Increase for faster panning on high-density screens.
  static const int8_t MOUSE_PAN_STEP = 60;

  // Touch coordinate space for Report ID 3 (0–4096 in both axes).
  // Pinch gesture: two fingers start close together at screen center,
  // then spread apart (zoom in) or contract (zoom out).
  static const uint16_t TOUCH_CENTER_X  = 2048; // horizontal center
  static const uint16_t TOUCH_CENTER_Y  = 2048; // vertical center
  static const uint16_t PINCH_START_RAD = 256;  // initial half-spread (fingers close)
  static const uint16_t PINCH_END_RAD   = 512;  // final   half-spread (fingers apart)
  static const uint16_t PINCH_STEP_PX   = 64;   // coordinate units moved per animation frame
  static const int      PINCH_STEP_MS   = 20;   // ms between frames (allows BLE to flush)

  void onConnect(NimBLEServer*, NimBLEConnInfo&) override {
    _connected = true;
    if (DEBUG) Serial.println("BLE connected");
  }

  void onDisconnect(NimBLEServer*, NimBLEConnInfo&, int reason) override {
    _connected = false;
    if (DEBUG) Serial.printf("BLE disconnected (reason %d), bonds in NVS: %d\n",
                             reason, NimBLEDevice::getNumBonds());
    // advertising restart handled automatically by advertiseOnDisconnect(true)
  }

  void onAuthenticationComplete(NimBLEConnInfo& conn_info) override {
    if (DEBUG) Serial.printf("Auth complete — bonded: %s, bonds stored: %d\n",
                             conn_info.isBonded() ? "yes" : "no",
                             NimBLEDevice::getNumBonds());
  }

  void send() {
    if (DEBUG) {
      Serial.printf("[BLE] send: connected=%d input=%s | mod=0x%02X keys=[%02X %02X %02X %02X %02X %02X]\n",
        (int)_connected, _input ? "ok" : "NULL",
        _rep.mod,
        _rep.keys[0], _rep.keys[1], _rep.keys[2],
        _rep.keys[3], _rep.keys[4], _rep.keys[5]);
    }
    if (_connected && _input) {
      _input->setValue((uint8_t*)&_rep, sizeof(_rep));
      _input->notify();
    }
  }

  // ---------------------------------------------------------------------------
  // Mouse helpers — send BLE HID mouse reports (Report ID 2)
  // ---------------------------------------------------------------------------

  // Dispatch a MOUSE_* / touch action code to the appropriate helper.
  void mouseAction(uint8_t action) {
    if (DEBUG) Serial.printf("[BLE] mouseAction: 0x%02X\n", action);
    switch (action) {
      case MOUSE_UP:             mouseDrag(0,              -MOUSE_PAN_STEP); break;
      case MOUSE_DOWN:           mouseDrag(0,               MOUSE_PAN_STEP); break;
      case MOUSE_LEFT:           mouseDrag(-MOUSE_PAN_STEP, 0);              break;
      case MOUSE_RIGHT:          mouseDrag( MOUSE_PAN_STEP, 0);              break;
      case MOUSE_SCROLL_UP:      mouseScroll( 1);                            break;
      case MOUSE_SCROLL_DOWN:    mouseScroll(-1);                            break;
      case MOUSE_CLICK:          mouseClick();                               break;
      case MOUSE_PINCH_ZOOM_IN:  touchPinch(true);                          break;
      case MOUSE_PINCH_ZOOM_OUT: touchPinch(false);                         break;
    }
  }

  // Simulate a short left-button drag in direction (dx, dy).
  // Android map apps (Waze, Google Maps) interpret click+drag as pan.
  //
  // After releasing the button we immediately send a counter-movement report
  // (same magnitude, opposite direction, button still up) to re-center the
  // cursor.  Map apps only pan while the button is held, so the return trip is
  // invisible to the application — but the cursor pointer ends up back where it
  // started, meaning this function can be called repeatedly without the cursor
  // ever drifting to a screen edge.
  void mouseDrag(int8_t dx, int8_t dy) {
    // Press left button with zero movement first so the host anchors the drag.
    _mouseRep = {0x01, 0, 0, 0};
    sendMouse();
    // Apply movement while the button is still held — app pans the map.
    _mouseRep.x = dx;
    _mouseRep.y = dy;
    sendMouse();
    // Release the button.
    _mouseRep = {0x00, 0, 0, 0};
    sendMouse();
    // Give the host a moment to finalise the drag gesture before the cursor
    // returns, so the recenter movement is not confused with the pan.
    delay(10);
    // Re-center: move cursor back by the same amount without pressing any button.
    // The button is already up so the app ignores this movement for panning; the
    // cursor silently returns to its original screen position.
    _mouseRep = {0x00, (int8_t)(-dx), (int8_t)(-dy), 0};
    sendMouse();
    _mouseRep = {0x00, 0, 0, 0};
    sendMouse();
  }

  // Send a scroll-wheel tick: positive = scroll up (zoom in), negative = down (zoom out).
  void mouseScroll(int8_t ticks) {
    _mouseRep = {0x00, 0, 0, ticks};
    sendMouse();
    delay(10);
    _mouseRep = {0x00, 0, 0, 0};
    sendMouse();
  }

  // Send a momentary left-button click (tap / select).
  void mouseClick() {
    _mouseRep = {0x01, 0, 0, 0};
    sendMouse();
    delay(10);
    _mouseRep = {0x00, 0, 0, 0};
    sendMouse();
  }

  // ---------------------------------------------------------------------------
  // Touch helpers — send BLE HID touch reports (Report ID 3)
  // ---------------------------------------------------------------------------

  // Simulate a two-finger pinch gesture at screen center.
  //   zoomIn=true  : fingers spread apart  → zoom in
  //   zoomIn=false : fingers come together → zoom out
  //
  // The gesture is animated over several frames so Android's gesture
  // recogniser can distinguish it from a tap.  Coordinates live in a
  // 0–4096 space; Android scales them to the physical display.
  void touchPinch(bool zoomIn) {
    // Starting and ending radii depend on direction.
    uint16_t r    = zoomIn ? PINCH_START_RAD : PINCH_END_RAD;
    int16_t  step = zoomIn ? (int16_t)PINCH_STEP_PX : -(int16_t)PINCH_STEP_PX;
    int      steps = (PINCH_END_RAD - PINCH_START_RAD) / PINCH_STEP_PX;
    // PINCH_END_RAD - PINCH_START_RAD (256) must be evenly divisible by
    // PINCH_STEP_PX (64) so the animation reaches the target radius exactly.
    // static_assert cannot reference non-constexpr member consts in all
    // compilers, so this is enforced by the constant definitions above.

    // Place both fingers on-screen.
    sendTouch(true,  0, TOUCH_CENTER_X - r, TOUCH_CENTER_Y,
              true,  1, TOUCH_CENTER_X + r, TOUCH_CENTER_Y, 2);
    delay(PINCH_STEP_MS);

    // Animate the spread / contract.
    for (int i = 0; i < steps; i++) {
      r = (uint16_t)((int16_t)r + step);
      sendTouch(true,  0, TOUCH_CENTER_X - r, TOUCH_CENTER_Y,
                true,  1, TOUCH_CENTER_X + r, TOUCH_CENTER_Y, 2);
      delay(PINCH_STEP_MS);
    }

    // Lift both fingers (contactCount = 0 signals end of gesture).
    sendTouch(false, 0, TOUCH_CENTER_X - r, TOUCH_CENTER_Y,
              false, 1, TOUCH_CENTER_X + r, TOUCH_CENTER_Y, 0);
  }

  // Send one touch report with two contacts.
  //   t1/t2    : tip switch (true = finger down)
  //   id1/id2  : contact IDs (0 and 1)
  //   x1/y1, x2/y2 : positions in 0–4096 coordinate space
  //   count    : active contact count (0 = all fingers lifted)
  void sendTouch(bool t1, uint8_t id1, uint16_t x1, uint16_t y1,
                 bool t2, uint8_t id2, uint16_t x2, uint16_t y2,
                 uint8_t count) {
    TouchReport rep;
    // flags byte bit layout (matches descriptor): bit 0=tip, bits 1-4=contact id, bits 5-7=padding(0)
    rep.c1.flags       = (t1 ? 0x01u : 0x00u) | ((id1 & 0x0Fu) << 1);
    rep.c1.x           = x1;
    rep.c1.y           = y1;
    rep.c2.flags       = (t2 ? 0x01u : 0x00u) | ((id2 & 0x0Fu) << 1); // same layout
    rep.c2.x           = x2;
    rep.c2.y           = y2;
    rep.contactCount   = count;
    if (DEBUG) {
      Serial.printf("[BLE] sendTouch: connected=%d touchInput=%s | "
                    "c1=[f=0x%02X x=%u y=%u] c2=[f=0x%02X x=%u y=%u] cnt=%u\n",
                    (int)_connected, _touchInput ? "ok" : "NULL",
                    rep.c1.flags, rep.c1.x, rep.c1.y,
                    rep.c2.flags, rep.c2.x, rep.c2.y, count);
    }
    if (_connected && _touchInput) {
      _touchInput->setValue((uint8_t*)&rep, sizeof(rep));
      _touchInput->notify();
    }
  }

  void sendMouse() {
    if (DEBUG) {
      Serial.printf("[BLE] sendMouse: connected=%d mouseInput=%s | btns=0x%02X x=%d y=%d scroll=%d\n",
        (int)_connected, _mouseInput ? "ok" : "NULL",
        _mouseRep.buttons, _mouseRep.x, _mouseRep.y, _mouseRep.scroll);
    }
    if (_connected && _mouseInput) {
      _mouseInput->setValue((uint8_t*)&_mouseRep, sizeof(_mouseRep));
      _mouseInput->notify();
    }
  }

  // Convert Arduino-Keyboard / BleKeyboard code → HID scan code + modifier bit
  static void toHID(uint8_t k, uint8_t& scan, uint8_t& mod) {
    scan = 0; mod = 0;
    if (k >= 0x80 && k <= 0x87) { mod = 1 << (k - 0x80); return; } // modifier keys
    if (k >= 32  && k <= 126)   {
      uint8_t e = pgm_read_byte(&_asciiToHid[k - 32]);
      if (e & 0x80) { mod = 0x02; scan = e & 0x7F; } else { scan = e; }
      return;
    }
    switch (k) {                    // special keys
      case 0xB0: scan=0x28; break;  // Return
      case 0xB1: scan=0x29; break;  // Esc
      case 0xB2: scan=0x2A; break;  // Backspace
      case 0xB3: scan=0x2B; break;  // Tab
      case 0xC1: scan=0x39; break;  // Caps Lock
      case 0xC2: scan=0x3A; break;  // F1
      case 0xC3: scan=0x3B; break;  // F2
      case 0xC4: scan=0x3C; break;  // F3
      case 0xC5: scan=0x3D; break;  // F4
      case 0xC6: scan=0x3E; break;  // F5
      case 0xC7: scan=0x3F; break;  // F6
      case 0xC8: scan=0x40; break;  // F7
      case 0xC9: scan=0x41; break;  // F8
      case 0xCA: scan=0x42; break;  // F9
      case 0xCB: scan=0x43; break;  // F10
      case 0xCC: scan=0x44; break;  // F11
      case 0xCD: scan=0x45; break;  // F12
      case 0xD1: scan=0x49; break;  // Insert
      case 0xD2: scan=0x4A; break;  // Home
      case 0xD3: scan=0x4B; break;  // Page Up
      case 0xD4: scan=0x4C; break;  // Delete
      case 0xD5: scan=0x4D; break;  // End
      case 0xD6: scan=0x4E; break;  // Page Down
      case 0xD7: scan=0x4F; break;  // Right Arrow
      case 0xD8: scan=0x50; break;  // Left Arrow
      case 0xD9: scan=0x51; break;  // Down Arrow
      case 0xDA: scan=0x52; break;  // Up Arrow
    }
  }
};
