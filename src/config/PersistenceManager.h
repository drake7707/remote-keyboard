#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <nvs_flash.h>
#include <nvs.h>
#include "buttons/KeyCodes.h"

extern const int DEBUG;
extern const bool LEGACY;

#define BLE_NAME_MAX_LEN 32
extern const char DEFAULT_BLE_NAME[];

// ---------------------------------------------------------------------------
// PersistenceManager -- stateless NVS I/O layer.
// Reads settings from NVS and returns them; writes settings to NVS when given
// values.  No in-memory state is kept here -- that lives in ConfigManager.
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

  // ---------------------------------------------------------------------------
  // NVS load functions -- read from NVS into caller-supplied storage
  // ---------------------------------------------------------------------------
  void    loadKeymaps(KeyEntry shortEntries[3][8], KeyEntry longEntries[3][8]);
  int     loadActiveKeymap();
  void    loadBleName(char* out, size_t len);
  bool    loadBatteryEnabled();
  bool    loadBLEPowerSaving();
  uint8_t loadMaxBLEConnections();

  // ---------------------------------------------------------------------------
  // NVS save functions -- write caller-supplied values to NVS
  // ---------------------------------------------------------------------------
  void saveKeymaps(const KeyEntry shortEntries[3][8], const KeyEntry longEntries[3][8]);
  void saveActiveKeymap(int slot);
  void saveBleName(const char* name);
  void saveBatteryEnabled(bool v);
  void saveBLEPowerSaving(bool v);
  void saveMaxBLEConnections(uint8_t v);

  // ---------------------------------------------------------------------------
  // NVS -- "clear bonds" flag (inherently stateless; no associated value)
  // ---------------------------------------------------------------------------
  void requestClearBonds();
  bool isClearBondsRequested();
  void clearClearBondsFlag();

  // ---------------------------------------------------------------------------
  // Utility
  // ---------------------------------------------------------------------------
  static int btnIndex(char key);
};
