#include "ConfigManager.h"

const char DEFAULT_BLE_NAME[] = "RemoteKeyboard";

void ConfigManager::begin(StatusLedManager *led, const char *firmwareVersion)
{
  webUI.begin(led, firmwareVersion);
}
