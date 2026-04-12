#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <nvs_flash.h>
#include <nvs.h>
#include "KeyCodes.h"

extern const int DEBUG;
extern const bool LEGACY;

#define BLE_NAME_MAX_LEN 32
extern const char DEFAULT_BLE_NAME[];

// ---------------------------------------------------------------------------
// PersistenceManager -- manages all NVS storage for keymaps, BLE name,
// battery settings, active keymap, and the "clear bonds" flag.
// ---------------------------------------------------------------------------

class PersistenceManager {
public:
  // Factory defaults -- mirror the original hard-coded behaviour
  static const uint8_t DEFAULT_SHORT[8];
  static const uint8_t DEFAULT_LONG[8];

  // Per-key target type
  enum KeyTarget : uint8_t {
    TARGET_SELECT = 0, // use the runtime target selector
    TARGET_HID    = 1, // send to a specific HID peer (or broadcast)
    TARGET_BTHOME = 2  // broadcast a BTHome advertisement
  };

  // One button action (short or long press)
  struct KeyEntry {
    uint8_t   key    = 0;
    KeyTarget target = TARGET_SELECT;
    char      mac[18] = {}; // HID peer MAC ("" = broadcast all)
  };

  // Load all settings from NVS in one call.
  void loadAll();

  // ---------------------------------------------------------------------------
  // NVS -- keymaps (3 slots)
  // ---------------------------------------------------------------------------
  void saveKeymap();

  // ---------------------------------------------------------------------------
  // NVS -- active keymap index (1, 2, or 3)
  // ---------------------------------------------------------------------------
  void setActiveKeymap(int slot);

  int getActiveKeymap() const { return _activeKeymap; }

  // ---------------------------------------------------------------------------
  // NVS -- BLE device name
  // ---------------------------------------------------------------------------
  void saveBleName();

  // ---------------------------------------------------------------------------
  // NVS -- battery enable flag
  // ---------------------------------------------------------------------------
  void saveBatteryEnabled();

  void saveBLEPowerSaving();

  void saveMaxBLEConnections();

  bool    isBatteryEnabled()     const { return _batteryEnabled; }
  bool    allowBLEPowerSaving()  const { return _blePowerSaving; }
  uint8_t getMaxBLEConnections() const { return _maxBLEConnections; }

  // ---------------------------------------------------------------------------
  // NVS -- "clear bonds" flag
  // ---------------------------------------------------------------------------
  void requestClearBonds();
  bool isClearBondsRequested();
  void clearClearBondsFlag();

  // ---------------------------------------------------------------------------
  // Keymap accessors
  // ---------------------------------------------------------------------------

  // Returns the short/long press entry for the active keymap (read-only).
  const KeyEntry& getShortEntry(int idx) const;
  const KeyEntry& getLongEntry(int idx)  const;

  // Returns a mutable reference to a specific keymap slot (used by web save handler).
  KeyEntry& rawShortEntry(int km, int idx);
  KeyEntry& rawLongEntry(int km, int idx);

  const char* getBleName() const { return _bleName; }
  void        setBleName(const char* name);

  void setBatteryEnabled(bool v)       { _batteryEnabled = v; }
  void setBlePowerSaving(bool v)       { _blePowerSaving = v; }
  void setMaxBLEConnections(uint8_t v) { _maxBLEConnections = v; }

  static int btnIndex(char key);

private:
  KeyEntry _shortEntries[3][8]  = {};
  KeyEntry _longEntries[3][8]   = {};
  int      _activeKeymap        = 1;
  bool     _batteryEnabled      = false;
  bool     _blePowerSaving      = false;
  uint8_t  _maxBLEConnections   = 1;
  char     _bleName[BLE_NAME_MAX_LEN + 1] = "RemoteKeyboard";

  // ---------------------------------------------------------------------------
  // Individual NVS loaders (called by loadAll)
  // ---------------------------------------------------------------------------
  void _loadKeymap();
  void _loadActiveKeymap();
  void _loadBleName();
  void _loadBatteryEnabled();
  void _loadBLEPowerSaving();
  void _loadMaxBLEConnections();
};
