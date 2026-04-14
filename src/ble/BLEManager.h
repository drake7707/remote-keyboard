#pragma once

#include <cstdio>
#include <cstring>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#include "HIDTypes.h"
#include "buttons/KeyCodes.h"
#include "BLEAdvertisingManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <map>
#include <algorithm>

extern const int DEBUG;

static const int MAX_CONCURRENT_CONNECTIONS = 2;

struct KbReport
{
  uint8_t modifier;
  uint8_t reserved;
  uint8_t keys[6];
};

// ---------------------------------------------------------------------------
// BLEManager — manages all BLE keyboard functionality
// NimBLE persists CCCD (notification subscription) in NVS per bonded peer,
// so keystrokes continue working after device resets without re-pairing.
// Security: bonding with Secure Connections, Just Works (no MITM, no pin).
// ---------------------------------------------------------------------------
class BLEManager : public NimBLEServerCallbacks
{
public:
  BLEManager(const char *manufacturer, uint8_t batteryLevel);

  void begin(const char *name, bool negotiatePowerSavingConnectionParameters, uint8_t maxConnections);

  void end();

  bool isConnected() const;

  std::vector<std::string> getConnections() const;

  // Return MAC address strings of all bonded devices stored in NVS.
  // Must be called while NimBLE is initialised (before end()).
  std::vector<std::string> getBondedAddresses() const;

  BLEAdvertisingManager& getAdvertisingManager();

  // Send a single key tap (press + immediate release).
  // Accepts any KEY_* constant, including media keys (KEY_MEDIA_PLAY_PAUSE, etc.).
  void write(const std::string &target, uint8_t key);

  // Hold a regular keyboard key down (accumulates modifiers / keys until releaseAll).
  void press(const std::string &target, uint8_t key);

  // Release all held keyboard keys.
  void releaseAll(const std::string &target);

  // Hold a media key down until releaseAllMedia() is called.
  // Accepts KEY_MEDIA_* constants (play/pause, stop, next, prev, vol up/down, mute).
  void pressMedia(const std::string &target, uint8_t key);

  // Release all held media keys.
  void releaseAllMedia(const std::string &target);

  // Update the BLE Battery Service level (0–100 %).
  // Notifies the connected host if a connection is active.
  void setBatteryLevel(uint8_t level);

  // Delete all stored BLE bonds from NVS
  static void clearAllBonds();

private:
  const char *_manufacturer;
  uint8_t _batteryLevel;
  bool _negotiatePowerSavingConnectionParameters = true;
  uint8_t _maxConnections = MAX_CONCURRENT_CONNECTIONS;

  std::map<std::string, uint16_t> _connections;
  NimBLEServer *_server = nullptr;
  NimBLEHIDDevice *_hid = nullptr;
  NimBLECharacteristic *_input = nullptr;
  NimBLECharacteristic *_inputCC = nullptr;
  KbReport _report = {};
  uint16_t _reportCC = 0;

  BLEAdvertisingManager _advManager;

  void onConnect(NimBLEServer *server, NimBLEConnInfo &connectionInfo) override;
  void onDisconnect(NimBLEServer *, NimBLEConnInfo &connectionInfo, int reason) override;
  void onAuthenticationComplete(NimBLEConnInfo &connectionInfo) override;

  void send(const std::string &target);
  void sendCC(const std::string &target);

  static bool     isMediaKey(uint8_t keyCode);
  static uint16_t mediaKeyToUsage(uint8_t keyCode);
  static void     toHID(uint8_t keyCode, uint8_t &scanCode, uint8_t &modifier);
  
  
};
