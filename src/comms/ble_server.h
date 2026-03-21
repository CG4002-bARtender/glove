#pragma once

#include <Arduino.h>
#include <string>
#include <NimBLEDevice.h>


class BleServer
{
public:
  BleServer() = default;

  void begin();
  bool isConnected() const { return _connected; }

  bool sendState(uint8_t stateId);

private:
  friend class BleServerCallbacks;

  void _onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc);
  void _onDisconnect();

  NimBLECharacteristic *_txChar    = nullptr;
  bool                   _connected = false;
};
