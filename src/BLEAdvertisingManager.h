#pragma once

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <map>
#include <string>
#include <algorithm>

extern const int DEBUG;

// ---------------------------------------------------------------------------
// BLEAdvertisingManager — sequential directed + undirected advertising cycle
//
// On each startCycle() call the sequence is:
//   1. Directed advertising to every unconnected bonded peer in NVS order,
//      DIRECTED_ADV_STEP_DURATION_MS per peer.
//   2. Undirected advertising for the remainder of the time budget
//      (infinite when no connection exists yet).
//
// When a connection already exists the total cycle is capped at
// MAX_ADVERTISING_DURATION_AFTER_ALREADY_CONNECTED_MS to save battery.
// ---------------------------------------------------------------------------
class BLEAdvertisingManager
{
public:
  // connections — live reference to BLEManager's connection map
  BLEAdvertisingManager(const std::map<std::string, uint16_t> &connections);

  // Configure advertising data and register the advertising-complete callback.
  // maxConnections is taken here because it is set via the web UI before begin()
  // is called, not at construction time.
  // Must be called once after the HID device is fully set up.
  void begin(NimBLEHIDDevice *hid, const char *deviceName, uint8_t maxConnections);

  // Start a fresh advertising cycle (directed bonds first, then undirected).
  // Resets internal state; safe to call at any time.
  void startCycle();

  // Advance to the next advertising step.
  // Invoked automatically by the advertising-complete callback; exposed so
  // callers can drive the cycle manually if needed.
  void advance();

private:
  const std::map<std::string, uint16_t> &_connections;
  uint8_t _maxConnections = 0;
  int _nextBondIdx = 0;
  uint32_t _advertisingCycleStartMs = 0;
};
