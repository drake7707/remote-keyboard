#include "config/ConfigManager.h"

void ConfigManager::begin(StatusLedManager* led, const char* firmwareVersion)
{
  webUI.begin(led, firmwareVersion);
}

void ConfigManager::loadAll()
{
  persistence.loadConfig(config);
}

void ConfigManager::saveConfig()
{
  persistence.saveConfig(config);
}

void ConfigManager::setActiveKeymap(int slot)
{
  if (slot < 1 || slot > 3)
    return;
  config.activeKeymap = slot;
  persistence.saveActiveKeymap(slot);
}

const KeyEntry& ConfigManager::getShortEntry(int idx) const
{
  static const KeyEntry empty{};
  int km = (config.activeKeymap >= 1 && config.activeKeymap <= 3) ? config.activeKeymap - 1 : 0;
  return (idx >= 0 && idx < 8) ? config.shortEntries[km][idx] : empty;
}

const KeyEntry& ConfigManager::getLongEntry(int idx) const
{
  static const KeyEntry empty{};
  int km = (config.activeKeymap >= 1 && config.activeKeymap <= 3) ? config.activeKeymap - 1 : 0;
  return (idx >= 0 && idx < 8) ? config.longEntries[km][idx] : empty;
}

KeyEntry& ConfigManager::rawShortEntry(int km, int idx)
{
  if (km < 0 || km >= 3)   km  = 0;
  if (idx < 0 || idx >= 8) idx = 0;
  return config.shortEntries[km][idx];
}

KeyEntry& ConfigManager::rawLongEntry(int km, int idx)
{
  if (km < 0 || km >= 3)   km  = 0;
  if (idx < 0 || idx >= 8) idx = 0;
  return config.longEntries[km][idx];
}

void ConfigManager::beginConfigAP(const std::vector<std::string>& bondList,
                                   int batVoltageMv, int batPercent)
{
  webUI.beginConfigAP(this, bondList, batVoltageMv, batPercent);
}
