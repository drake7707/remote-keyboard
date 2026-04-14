#pragma once

#include <vector>
#include <string>
#include "PersistenceManager.h"
#include "WebUIConfigManager.h"

// ---------------------------------------------------------------------------
// ConfigManager -- ties PersistenceManager and WebUIConfigManager together.
// Preserves the original public API so that callers (main.cpp) need no changes.
// ---------------------------------------------------------------------------

class ConfigManager {
public:
  // Re-export types from PersistenceManager for callers that use
  // ConfigManager::KeyTarget, ConfigManager::KeyEntry, etc.
  using KeyTarget = PersistenceManager::KeyTarget;
  using KeyEntry  = PersistenceManager::KeyEntry;

  static constexpr PersistenceManager::KeyTarget TARGET_SELECT = PersistenceManager::TARGET_SELECT;
  static constexpr PersistenceManager::KeyTarget TARGET_HID    = PersistenceManager::TARGET_HID;
  static constexpr PersistenceManager::KeyTarget TARGET_BTHOME = PersistenceManager::TARGET_BTHOME;

  // Inject the StatusLedManager so web handlers can signal progress via LED,
  // and the firmware version string to display in the web config UI.
  void begin(StatusLedManager* led, const char* firmwareVersion);

  // Load all settings from NVS in one call.
  void loadAll() { persistence.loadAll(); }

  // ---------------------------------------------------------------------------
  // Delegated to PersistenceManager
  // ---------------------------------------------------------------------------
  void    saveKeymap()            { persistence.saveKeymap(); }
  void    setActiveKeymap(int s)  { persistence.setActiveKeymap(s); }
  int     getActiveKeymap() const { return persistence.getActiveKeymap(); }
  void    saveBleName()           { persistence.saveBleName(); }
  void    saveBatteryEnabled()    { persistence.saveBatteryEnabled(); }
  void    saveBLEPowerSaving()    { persistence.saveBLEPowerSaving(); }
  void    saveMaxBLEConnections() { persistence.saveMaxBLEConnections(); }
  bool    isBatteryEnabled()     const { return persistence.isBatteryEnabled(); }
  bool    allowBLEPowerSaving()  const { return persistence.allowBLEPowerSaving(); }
  uint8_t getMaxBLEConnections() const { return persistence.getMaxBLEConnections(); }
  void    requestClearBonds()    { persistence.requestClearBonds(); }
  bool    isClearBondsRequested(){ return persistence.isClearBondsRequested(); }
  void    clearClearBondsFlag()  { persistence.clearClearBondsFlag(); }

  const KeyEntry& getShortEntry(int idx) const { return persistence.getShortEntry(idx); }
  const KeyEntry& getLongEntry(int idx)  const { return persistence.getLongEntry(idx); }
  const char*     getBleName()           const { return persistence.getBleName(); }

  static int btnIndex(char key) { return PersistenceManager::btnIndex(key); }

  // ---------------------------------------------------------------------------
  // Delegated to WebUIConfigManager
  // ---------------------------------------------------------------------------
  void beginConfigAP(const std::vector<std::string>& bondList,
                     int batVoltageMv = -1, int batPercent = -1)
  {
    webUI.beginConfigAP(&persistence, bondList, batVoltageMv, batPercent);
  }
  void handleClient()           { webUI.handleClient(); }
  void endConfigAP()            { webUI.endConfigAP(); }
  bool isExitRequested() const  { return webUI.isExitRequested(); }
  void setExitRequested(bool v) { webUI.setExitRequested(v); }

  // ---------------------------------------------------------------------------
  // Sub-managers (exposed for direct access if needed)
  // ---------------------------------------------------------------------------
  PersistenceManager  persistence;
  WebUIConfigManager  webUI;
};
