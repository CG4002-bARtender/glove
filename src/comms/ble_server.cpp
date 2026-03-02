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

// ── Gesture framing ───────────────────────────────────────────────────────────

bool BleServer::sendGestureStart(uint16_t count)
{
  if (!_connected) return false;
  uint8_t buf[3] = {
    BLE_PKT_GESTURE_START,
    (uint8_t)(count & 0xFF),
    (uint8_t)(count >> 8)
  };
  _txChar->setValue(buf, sizeof(buf));
  _txChar->notify();
  return true;
}

// Sends n samples packed into one GESTURE_SAMPLE notify.
// Layout: [0x02][n][n × 22 bytes]
bool BleServer::sendGestureSample(const uint8_t *data, size_t n)
{
  if (!_connected || n == 0) return false;
  const size_t pktLen = 2 + n * BLE_SAMPLE_BYTES;
  uint8_t buf[2 + BLE_SAMPLES_PER_PKT * BLE_SAMPLE_BYTES];
  buf[0] = BLE_PKT_GESTURE_SAMPLE;
  buf[1] = static_cast<uint8_t>(n);
  memcpy(buf + 2, data, n * BLE_SAMPLE_BYTES);
  _txChar->setValue(buf, pktLen);
  _txChar->notify();
  return true;
}

bool BleServer::sendGestureEnd()
{
  if (!_connected) return false;
  uint8_t buf[1] = { BLE_PKT_GESTURE_END };
  _txChar->setValue(buf, sizeof(buf));
  _txChar->notify();
  return true;
}

// Sends a full gesture window: START + batched SAMPLEs + END.
// Packs BLE_SAMPLES_PER_PKT samples per notify to keep packet count low.
bool BleServer::sendGestureWindow(const uint8_t *data, size_t totalSamples)
{
  if (!_connected) return false;

  sendGestureStart(static_cast<uint16_t>(totalSamples));

  size_t sent = 0;
  while (sent < totalSamples) {
    const size_t batch = min(totalSamples - sent, BLE_SAMPLES_PER_PKT);
    sendGestureSample(data + sent * BLE_SAMPLE_BYTES, batch);
    sent += batch;
  }

  sendGestureEnd();
  return true;
}

// ── Low-level helpers ─────────────────────────────────────────────────────────

bool BleServer::send(const std::string &msg)
{
  if (!_connected) return false;
  _txChar->setValue(msg);
  _txChar->notify();
  return true;
}

bool BleServer::sendRaw(const uint8_t *data, size_t len)
{
  if (!_connected) return false;
  _txChar->setValue(data, len);
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
