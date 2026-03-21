#include "ble_server.h"
#include "../config.h"
#include <string.h>

class BleServerCallbacks : public NimBLEServerCallbacks
{
  BleServer *_owner;
public:
  explicit BleServerCallbacks(BleServer *o) : _owner(o) {}

  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override
  {
    _owner->_onConnect(pServer, desc);
  }

  void onDisconnect(NimBLEServer *) override
  {
    _owner->_onDisconnect();
  }
};

void BleServer::begin()
{
  NimBLEDevice::init(config::ble::DEVICE_NAME);
  NimBLEDevice::setMTU(config::ble::MTU);

  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new BleServerCallbacks(this));

  NimBLEService *pService = pServer->createService(config::ble::SERVICE_UUID);
  _txChar = pService->createCharacteristic(config::ble::CHAR_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
  pService->start();

  NimBLEAdvertising *pAdv = NimBLEDevice::getAdvertising();
  pAdv->addServiceUUID(config::ble::SERVICE_UUID);
  pAdv->setScanResponse(true);
  pAdv->start();

  DEBUG_PRINTF("[BLE] Advertising as '%s'\n", config::ble::DEVICE_NAME);
}

// ── State event ───────────────────────────────────────────────────────────────

bool BleServer::sendState(uint8_t stateId)
{
  if (!_connected) return false;
  const uint8_t buf[1] = { stateId };
  _txChar->setValue(buf, sizeof(buf));
  _txChar->notify();
  return true;
}

// ── Connection callbacks ──────────────────────────────────────────────────────

void BleServer::_onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
  _connected = true;
  DEBUG_PRINTLN("[BLE] Client connected");
  pServer->updateConnParams(
    desc->conn_handle,
    config::ble::CONN_MIN_INTERVAL,
    config::ble::CONN_MAX_INTERVAL,
    config::ble::CONN_LATENCY,
    config::ble::CONN_TIMEOUT
  );
}

void BleServer::_onDisconnect()
{
  _connected = false;
  DEBUG_PRINTLN("[BLE] Client disconnected, restarting advertising...");
  NimBLEDevice::startAdvertising();
}
