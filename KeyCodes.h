#pragma once

// ---------------------------------------------------------------------------
// Key code constants
// Same numeric values as BleKeyboard.h / Arduino Keyboard.h so NVS keymap
// data stored by earlier firmware remains valid.
// ---------------------------------------------------------------------------
#define KEY_LEFT_CTRL    0x80
#define KEY_LEFT_SHIFT   0x81
#define KEY_LEFT_ALT     0x82
#define KEY_LEFT_GUI     0x83
#define KEY_RIGHT_CTRL   0x84
#define KEY_RIGHT_SHIFT  0x85
#define KEY_RIGHT_ALT    0x86
#define KEY_RIGHT_GUI    0x87
#define KEY_RETURN       0xB0
#define KEY_ESC          0xB1
#define KEY_BACKSPACE    0xB2
#define KEY_TAB          0xB3
#define KEY_INSERT       0xD1
#define KEY_HOME         0xD2
#define KEY_PAGE_UP      0xD3
#define KEY_DELETE       0xD4
#define KEY_END          0xD5
#define KEY_PAGE_DOWN    0xD6
#define KEY_RIGHT_ARROW  0xD7
#define KEY_LEFT_ARROW   0xD8
#define KEY_DOWN_ARROW   0xD9
#define KEY_UP_ARROW     0xDA
#define KEY_CAPS_LOCK    0xC1
#define KEY_F1   0xC2
#define KEY_F2   0xC3
#define KEY_F3   0xC4
#define KEY_F4   0xC5
#define KEY_F5   0xC6
#define KEY_F6   0xC7
#define KEY_F7   0xC8
#define KEY_F8   0xC9
#define KEY_F9   0xCA
#define KEY_F10  0xCB
#define KEY_F11  0xCC
#define KEY_F12  0xCD

// ---------------------------------------------------------------------------
// Mouse action codes (0xE0–0xE6).
// These are not real HID scan codes — BLEManager intercepts them in write()
// and sends BLE HID mouse reports (Report ID 2) instead of keyboard reports.
// ---------------------------------------------------------------------------
#define MOUSE_UP          0xE0  // move cursor up    (pan map north)
#define MOUSE_DOWN        0xE1  // move cursor down  (pan map south)
#define MOUSE_LEFT        0xE2  // move cursor left  (pan map west)
#define MOUSE_RIGHT       0xE3  // move cursor right (pan map east)
#define MOUSE_SCROLL_UP   0xE4  // scroll wheel up   (zoom in)
#define MOUSE_SCROLL_DOWN 0xE5  // scroll wheel down (zoom out)
#define MOUSE_CLICK       0xE6  // left button click (tap / select)
