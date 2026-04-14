#include "config/PersistenceManager.h"

// ---------------------------------------------------------------------------
// loadConfig -- read all settings from NVS into 'config' in one pass.
// ---------------------------------------------------------------------------
void PersistenceManager::loadConfig(Config &config)
{
  // --- Keymaps (3 slots x 8 buttons) ---
  const char *namespaces[3] = {"keymap", "keymap2", "keymap3"};
  for (int keymap = 0; keymap < 3; keymap++)
  {
    nvs_handle_t nvsHandle;
    bool opened = (nvs_open(namespaces[keymap], NVS_READONLY, &nvsHandle) == ESP_OK);
    for (int i = 0; i < 8; i++)
    {
      char nvsKey[8];

      snprintf(nvsKey, sizeof(nvsKey), "s%d", i);
      uint8_t value = Config::DEFAULT_SHORT[i];
      if (opened)
        nvs_get_u8(nvsHandle, nvsKey, &value);
      config.shortEntries[keymap][i].key = value;

      snprintf(nvsKey, sizeof(nvsKey), "l%d", i);
      value = Config::DEFAULT_LONG[i];
      if (opened)
        nvs_get_u8(nvsHandle, nvsKey, &value);
      config.longEntries[keymap][i].key = value;

      snprintf(nvsKey, sizeof(nvsKey), "st%d", i);
      uint8_t rawTarget = TARGET_SELECT;
      if (opened)
        nvs_get_u8(nvsHandle, nvsKey, &rawTarget);
      if (rawTarget > TARGET_BTHOME)
        rawTarget = TARGET_SELECT;
      config.shortEntries[keymap][i].target = (KeyTarget)rawTarget;

      snprintf(nvsKey, sizeof(nvsKey), "lt%d", i);
      rawTarget = TARGET_SELECT;
      if (opened)
        nvs_get_u8(nvsHandle, nvsKey, &rawTarget);
      if (rawTarget > TARGET_BTHOME)
        rawTarget = TARGET_SELECT;
      config.longEntries[keymap][i].target = (KeyTarget)rawTarget;

      snprintf(nvsKey, sizeof(nvsKey), "sm%d", i);
      config.shortEntries[keymap][i].mac[0] = '\0';
      if (opened)
      {
        size_t macLen = sizeof(config.shortEntries[keymap][i].mac);
        nvs_get_str(nvsHandle, nvsKey, config.shortEntries[keymap][i].mac, &macLen);
      }

      snprintf(nvsKey, sizeof(nvsKey), "lm%d", i);
      config.longEntries[keymap][i].mac[0] = '\0';
      if (opened)
      {
        size_t macLen = sizeof(config.longEntries[keymap][i].mac);
        nvs_get_str(nvsHandle, nvsKey, config.longEntries[keymap][i].mac, &macLen);
      }
    }
    if (opened)
      nvs_close(nvsHandle);
  }
  if (DEBUG)
  {
    printf("[CONFIG] Keymaps loaded from NVS:\n");
    for (int keymap = 0; keymap < 3; keymap++)
    {
      printf("[CONFIG]   Keymap %d:\n", keymap + 1);
      for (int i = 0; i < 8; i++)
        printf("[CONFIG]     btn%d  short=%d(tgt=%d mac=%s)  long=%d(tgt=%d mac=%s)\n",
               i + 1,
               config.shortEntries[keymap][i].key, (int)config.shortEntries[keymap][i].target, config.shortEntries[keymap][i].mac,
               config.longEntries[keymap][i].key, (int)config.longEntries[keymap][i].target, config.longEntries[keymap][i].mac);
    }
  }

  // --- Active keymap index ---
  {
    nvs_handle_t nvsHandle;
    uint8_t savedSlot = 1;
    if (nvs_open("sys", NVS_READONLY, &nvsHandle) == ESP_OK)
    {
      nvs_get_u8(nvsHandle, "activekm", &savedSlot);
      nvs_close(nvsHandle);
    }
    config.activeKeymap = (savedSlot >= 1 && savedSlot <= 3) ? (int)savedSlot : 1;
  }
  if (DEBUG)
    printf("[CONFIG] Active keymap loaded: %d\n", config.activeKeymap);

  // --- BLE name ---
  strncpy(config.bleName, DEFAULT_BLE_NAME, sizeof(config.bleName) - 1);
  config.bleName[sizeof(config.bleName) - 1] = '\0';
  {
    nvs_handle_t nvsHandle;
    if (nvs_open("config", NVS_READONLY, &nvsHandle) == ESP_OK)
    {
      size_t len = sizeof(config.bleName);
      nvs_get_str(nvsHandle, "blename", config.bleName, &len);
      nvs_close(nvsHandle);
    }
  }
  if (DEBUG)
    printf("[CONFIG] BLE name loaded: %s\n", config.bleName);

  // --- Flags from "sys" namespace ---
  {
    nvs_handle_t nvsHandle;
    if (nvs_open("sys", NVS_READONLY, &nvsHandle) == ESP_OK)
    {
      uint8_t flag = 0;
      nvs_get_u8(nvsHandle, "baten", &flag);
      config.batteryEnabled = (flag != 0);

      flag = 0;
      nvs_get_u8(nvsHandle, "blepsen", &flag);
      config.blePowerSaving = (flag != 0);

      uint8_t value = 1;
      nvs_get_u8(nvsHandle, "maxbleconn", &value);
      config.maxBLEConnections = (value >= 1 && value <= 3) ? value : 1;

      nvs_close(nvsHandle);
    }
  }
  if (DEBUG)
  {
    printf("[CONFIG] Battery enabled: %s\n", config.batteryEnabled ? "yes" : "no");
    printf("[CONFIG] BLE power saving: %s\n", config.blePowerSaving ? "yes" : "no");
    printf("[CONFIG] Max BLE connections: %d\n", config.maxBLEConnections);
  }
}

// ---------------------------------------------------------------------------
// saveConfig -- write all settings from 'config' to NVS in one pass.
// ---------------------------------------------------------------------------
void PersistenceManager::saveConfig(const Config &config)
{
  // --- Keymaps ---
  const char *namespaces[3] = {"keymap", "keymap2", "keymap3"};
  for (int keymap = 0; keymap < 3; keymap++)
  {
    nvs_handle_t nvsHandle;
    if (nvs_open(namespaces[keymap], NVS_READWRITE, &nvsHandle) != ESP_OK)
      continue;
    for (int i = 0; i < 8; i++)
    {
      char nvsKey[8];
      snprintf(nvsKey, sizeof(nvsKey), "s%d",  i); nvs_set_u8( nvsHandle, nvsKey, config.shortEntries[keymap][i].key);
      snprintf(nvsKey, sizeof(nvsKey), "l%d",  i); nvs_set_u8( nvsHandle, nvsKey, config.longEntries[keymap][i].key);
      snprintf(nvsKey, sizeof(nvsKey), "st%d", i); nvs_set_u8( nvsHandle, nvsKey, (uint8_t)config.shortEntries[keymap][i].target);
      snprintf(nvsKey, sizeof(nvsKey), "lt%d", i); nvs_set_u8( nvsHandle, nvsKey, (uint8_t)config.longEntries[keymap][i].target);
      snprintf(nvsKey, sizeof(nvsKey), "sm%d", i); nvs_set_str(nvsHandle, nvsKey, config.shortEntries[keymap][i].mac);
      snprintf(nvsKey, sizeof(nvsKey), "lm%d", i); nvs_set_str(nvsHandle, nvsKey, config.longEntries[keymap][i].mac);
    }
    nvs_commit(nvsHandle);
    nvs_close(nvsHandle);
  }
  if (DEBUG)
    printf("[CONFIG] Keymaps saved to NVS.\n");

  // --- BLE name ---
  {
    nvs_handle_t nvsHandle;
    if (nvs_open("config", NVS_READWRITE, &nvsHandle) == ESP_OK)
    {
      nvs_set_str(nvsHandle, "blename", config.bleName);
      nvs_commit(nvsHandle);
      nvs_close(nvsHandle);
    }
  }
  if (DEBUG)
    printf("[CONFIG] BLE name saved: %s\n", config.bleName);

  // --- Active keymap + flags ---
  {
    nvs_handle_t nvsHandle;
    if (nvs_open("sys", NVS_READWRITE, &nvsHandle) == ESP_OK)
    {
      nvs_set_u8(nvsHandle, "activekm",   (uint8_t)config.activeKeymap);
      nvs_set_u8(nvsHandle, "baten",      config.batteryEnabled  ? 1 : 0);
      nvs_set_u8(nvsHandle, "blepsen",    config.blePowerSaving  ? 1 : 0);
      nvs_set_u8(nvsHandle, "maxbleconn", config.maxBLEConnections);
      nvs_commit(nvsHandle);
      nvs_close(nvsHandle);
    }
  }
  if (DEBUG)
  {
    printf("[CONFIG] Active keymap saved: %d\n",    config.activeKeymap);
    printf("[CONFIG] Battery enabled saved: %s\n",  config.batteryEnabled  ? "yes" : "no");
    printf("[CONFIG] BLE power saving saved: %s\n", config.blePowerSaving  ? "yes" : "no");
    printf("[CONFIG] Max BLE connections saved: %d\n", config.maxBLEConnections);
  }
}

// ---------------------------------------------------------------------------
// saveActiveKeymap -- lightweight runtime save (button-combo keymap switch).
// ---------------------------------------------------------------------------
void PersistenceManager::saveActiveKeymap(int slot)
{
  nvs_handle_t nvsHandle;
  if (nvs_open("sys", NVS_READWRITE, &nvsHandle) == ESP_OK)
  {
    nvs_set_u8(nvsHandle, "activekm", (uint8_t)slot);
    nvs_commit(nvsHandle);
    nvs_close(nvsHandle);
  }
  if (DEBUG)
    printf("[CONFIG] Active keymap saved: %d\n", slot);
}

// ---------------------------------------------------------------------------
// Clear-bonds flag
// ---------------------------------------------------------------------------
void PersistenceManager::requestClearBonds()
{
  nvs_handle_t nvsHandle;
  if (nvs_open("sys", NVS_READWRITE, &nvsHandle) == ESP_OK)
  {
    nvs_set_u8(nvsHandle, "clrbond", 1);
    nvs_commit(nvsHandle);
    nvs_close(nvsHandle);
  }
}

bool PersistenceManager::isClearBondsRequested()
{
  nvs_handle_t nvsHandle;
  uint8_t flag = 0;
  if (nvs_open("sys", NVS_READONLY, &nvsHandle) == ESP_OK)
  {
    nvs_get_u8(nvsHandle, "clrbond", &flag);
    nvs_close(nvsHandle);
  }
  return flag != 0;
}

void PersistenceManager::clearClearBondsFlag()
{
  nvs_handle_t nvsHandle;
  if (nvs_open("sys", NVS_READWRITE, &nvsHandle) == ESP_OK)
  {
    nvs_erase_key(nvsHandle, "clrbond");
    nvs_commit(nvsHandle);
    nvs_close(nvsHandle);
  }
}
