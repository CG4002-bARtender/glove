#include <Arduino.h>
#include "../config.h"
#include <NimBLEDevice.h>

// BLE UART-over-GATT UUIDs (Nordic UART Service)
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_TX        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // ESP32 -> laptop (notify)
#define CHAR_UUID_RX        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // laptop -> ESP32 (write)

static NimBLECharacteristic *pTxChar = nullptr;
static bool deviceConnected = false;
static uint32_t counter = 0;

class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override
  {
    deviceConnected = true;
    DEBUG_PRINTLN("[BLE] Client connected");
    // Negotiate connection parameters acceptable to Chrome's Web Bluetooth:
    // 20–40 ms interval, 0 latency, 6 s supervision timeout
    pServer->updateConnParams(desc->conn_handle, 16, 32, 0, 600);
  }

  void onDisconnect(NimBLEServer *pServer) override
  {
    deviceConnected = false;
    DEBUG_PRINTLN("[BLE] Client disconnected, restarting advertising...");
    NimBLEDevice::startAdvertising();
  }
};

class RxCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pChar) override
  {
    std::string val = pChar->getValue();
    if (!val.empty())
    {
      DEBUG_PRINT("[BLE] Received: ");
      DEBUG_PRINTLN(val.c_str());
    }
  }
};

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== BLE Test Script ===");

  NimBLEDevice::init("ESP32-Glove");

  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  // TX characteristic: ESP32 notifies laptop
  pTxChar = pService->createCharacteristic(
    CHAR_UUID_TX,
    NIMBLE_PROPERTY::NOTIFY
  );

  // RX characteristic: laptop writes to ESP32
  NimBLECharacteristic *pRxChar = pService->createCharacteristic(
    CHAR_UUID_RX,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  pRxChar->setCallbacks(new RxCallbacks());

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  DEBUG_PRINTLN("[BLE] Advertising as 'ESP32-Glove'");
}

void loop()
{
  if (deviceConnected)
  {
    std::string msg = "ping " + std::to_string(counter++);
    pTxChar->setValue(msg);
    pTxChar->notify();
    DEBUG_PRINTF("[BLE] Sent: %s\n", msg.c_str());
  }
  delay(1000);
}
