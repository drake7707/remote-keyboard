#include "BLEAdvertisingManager.h"

BLEAdvertisingManager::BLEAdvertisingManager(const std::map<std::string, uint16_t> &connections)
    : _connections(connections) {}

void BLEAdvertisingManager::begin(NimBLEHIDDevice *hidDevice, const char *deviceName, uint8_t maxConnections)
{
  _maxConnections = maxConnections;
  _hid = hidDevice;
  _deviceName = deviceName;

  configureHIDAdvertising();

  // When advertising ends (directed timeout, connection, or explicit stop)
  // NimBLE fires this callback — we use it to step to the next bond or fall
  // back to undirected advertising.
    NimBLEDevice::getAdvertising()->setAdvertisingCompleteCallback(
      [this](NimBLEAdvertising *) { onAdvertisementComplete(); });
}

void BLEAdvertisingManager::configureHIDAdvertising()
{
  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

  // Build the primary advertisement payload explicitly so we can switch
  // back to it cleanly after a BTHome broadcast (NimBLE has no public API
  // to clear custom advertisement data once set).
  NimBLEAdvertisementData advertisementData;
  // 0x06 = LE General Discoverable Mode | BR/EDR Not Supported
  advertisementData.setFlags(0x06);
  // Appearance: HID Keyboard (0x03C1).  Full AD structure layout:
  //   [0x03]  length  = 3 bytes follow (type + 2-byte value)
  //   [0x19]  type    = Appearance
  //   [0xC1]  value LSB of 0x03C1 (HID Keyboard appearance)
  //   [0x03]  value MSB of 0x03C1
  uint8_t appearance[] = {0x03, 0x19, 0xC1, 0x03};
  advertisementData.addData(appearance, sizeof(appearance));
  advertisementData.addServiceUUID(_hid->getHidService()->getUUID());
  advertising->setAdvertisementData(advertisementData);

  // Include device name in scan response so Android/Windows show it in
  // the pairing list and recognise it as a keyboard.
  NimBLEAdvertisementData scanResponseData;
  scanResponseData.setName(_deviceName);
  advertising->setScanResponseData(scanResponseData);
}

void BLEAdvertisingManager::startCycle()
{
  if (_connections.size() >= _maxConnections)
  {
    if (DEBUG)
      printf("[BLE ADV] Connection limit reached (%d), not advertising\n", _maxConnections);
    return;
  }

  _nextBondIndex = 0;
  _advertisingCycleStartMs = pdTICKS_TO_MS(xTaskGetTickCount());

  if (DEBUG)
    printf("[BLE ADV] Starting advertising cycle with %d connection%s\n",
           (int)_connections.size(), _connections.size() == 1 ? "" : "s");

  advance();
}

void BLEAdvertisingManager::onAdvertisementComplete()
{
  // If we just finished a BTHome broadcast, restore HID advertising data
  // and restart the normal advertising cycle only if it was running before
  // the broadcast was triggered.
  if (_btHomeBroadcastActive)
  {
    _btHomeBroadcastActive = false;
    configureHIDAdvertising();
    if (_restoreHIDAfterBTHome)
    {
      if (DEBUG)
        printf("[BLE ADV] BTHome broadcast complete, restarting HID advertising cycle\n");
      startCycle();
    }
    else
    {
      if (DEBUG)
        printf("[BLE ADV] BTHome broadcast complete, HID advertising was not running — not restarting\n");
    }
    return;
  }
  else
  {
    // Normal advertising complete (directed adv timeout, connection, or explicit stop) — step to the next one.
    advance();
  }
}

void BLEAdvertisingManager::advance()
{
  if (_connections.size() >= _maxConnections)
  {
    if (DEBUG)
      printf("[BLE ADV] Connection limit reached (%d), stopping advertising cycle\n", _maxConnections);
    return;
  }

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

  // Guard: onConnect() calls startCycle() which may already have started a
  // new advertising step before this callback fires for the previous directed
  // advertisement that just connected.
  if (advertising->isAdvertising())
  {
    if (DEBUG)
      printf("[BLE ADV] Advertising already active, skipping advance\n");
    return;
  }

  // Calculate remaining time budget.
  // remainingBudgetMs == 0 means "infinite" (no connection exists yet).
  uint32_t remainingBudgetMs = 0;
  if (!_connections.empty())
  {
    // Unsigned subtraction is intentional: correct even if xTaskGetTickCount()
    // wraps (every ~49.7 days), since the advertising budget is only 60 s.
    uint32_t elapsed = pdTICKS_TO_MS(xTaskGetTickCount()) - _advertisingCycleStartMs;
    if (elapsed >= MAX_ADVERTISING_DURATION_AFTER_ALREADY_CONNECTED_MS)
    {
      if (DEBUG)
        printf("[BLE ADV] Advertising budget exhausted after %lu ms, stopping\n", elapsed);
      return;
    }
    remainingBudgetMs = MAX_ADVERTISING_DURATION_AFTER_ALREADY_CONNECTED_MS - elapsed;
  }

  // Try each unconnected bonded peer in order, starting from _nextBondIndex.
  int bondCount = NimBLEDevice::getNumBonds();
  for (int i = _nextBondIndex; i < bondCount; i++)
  {
    NimBLEAddress addr = NimBLEDevice::getBondedAddress(i);
    std::string peerAddress = addr.toString();

    if (_connections.find(peerAddress) != _connections.end())
      continue; // already connected, skip to next bond

    _nextBondIndex = i + 1;
    uint32_t directedDuration = (remainingBudgetMs == 0)
                                    ? DIRECTED_ADV_STEP_DURATION_MS
                                    : std::min(remainingBudgetMs, DIRECTED_ADV_STEP_DURATION_MS);

    if (DEBUG)
      printf("[BLE ADV] Start directed advertising to bonded peer %s for %lu ms\n", peerAddress.c_str(), directedDuration);

    advertising->start(directedDuration, &addr);
    return;
  }

  // All bonds tried (or no bonds at all) — fall back to undirected advertising.
  if (DEBUG)
    printf("[BLE ADV] All bonds tried, starting undirected advertising for %lu ms\n", remainingBudgetMs);

  advertising->start(remainingBudgetMs);
}

bool BLEAdvertisingManager::isAdvertising() const
{
  return NimBLEDevice::getAdvertising()->isAdvertising();
}

void BLEAdvertisingManager::broadcastBTHomeButtonPress(uint8_t eventType, uint8_t button)
{
  if (button < 1 || button > 8)
  {
    if (DEBUG)
      printf("[BLE ADV] BTHome: invalid button %d (must be 1–8)\n", button);
    return;
  }

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

  bool wasAdvertising = advertising->isAdvertising();
  if (wasAdvertising)
    advertising->stop();

  // Record whether HID advertising was running so we can restore it after the
  // broadcast.  If a BTHome broadcast is already in flight (rapid button press)
  // we keep the flag from the first press so the original intent is preserved:
  // if HID was running when the first press fired, it should still be restarted
  // once all BTHome broadcasts are done.
  if (!_btHomeBroadcastActive)
    _restoreHIDAfterBTHome = wasAdvertising;

  // ---------------------------------------------------------------------------
  // BTHome v2 service data payload
  //
  // Byte 0   — device info: version=2, trigger-based=1, no encryption → 0x44
  //              bit 0   : encryption (0 = none)
  //              bit 1   : reserved
  //              bit 2   : trigger-based device (1 = event/trigger)
  //              bit 3–4 : reserved
  //              bit 5–7 : BTHome version (010 = v2)
  //
  // Bytes 1–2  — Packet ID object (object ID 0x00, 1-byte uint8 value)
  //              Incremented on every new event so Home Assistant can
  //              distinguish distinct events from duplicate re-transmissions
  //              of the same advertisement.
  //
  // Bytes 3–18 — 8 × Button object (object ID 0x3A, 1-byte value)
  //              Only the slot for the pressed button carries a non-zero event;
  //              all other slots carry 0x00 (None).  Advertising all 8 slots
  //              from the first packet lets Home Assistant create all 8 button
  //              entities immediately on discovery.
  // ---------------------------------------------------------------------------
  static const uint8_t DEVICE_INFO = 0x44;   // v2, trigger-based, no encryption
  static const uint8_t PACKET_ID_OBJ = 0x00; // BTHome Packet ID object ID
  static const uint8_t BUTTON_OBJ_ID = 0x3A; // BTHome Button object ID
  // Payload: 1 device-info byte + 1 packet-id object (ID + value) + 8 button objects (ID + value each).
  static const int BTHOME_PAYLOAD_SIZE = 1 + 2 + 8 * 2;

  _btHomePacketId++; // increment before each new broadcast

  uint8_t payload[BTHOME_PAYLOAD_SIZE];
  payload[0] = DEVICE_INFO;
  payload[1] = PACKET_ID_OBJ;
  payload[2] = _btHomePacketId;
  for (int i = 0; i < 8; i++)
  {
    payload[3 + i * 2] = BUTTON_OBJ_ID;
    payload[3 + i * 2 + 1] = (i == (button - 1)) ? eventType : BTHOME_BUTTON_NONE;
  }

  // BTHome uses 16-bit service data UUID 0xFCD2.
  NimBLEAdvertisementData btHomeData;
  btHomeData.setServiceData(NimBLEUUID((uint16_t)0xFCD2),
                            std::string(reinterpret_cast<char *>(payload), sizeof(payload)));
  advertising->setAdvertisementData(btHomeData);

  // Clear scan response for the BTHome broadcast — HA needs only the service data.
  advertising->setScanResponseData(NimBLEAdvertisementData());

  _btHomeBroadcastActive = true;

  if (DEBUG)
    printf("[BLE ADV] BTHome: broadcasting button %d event 0x%02X packet_id %d for %lu ms\n",
           button, eventType, _btHomePacketId, BTHOME_BROADCAST_DURATION_MS);

  advertising->start(BTHOME_BROADCAST_DURATION_MS);
}