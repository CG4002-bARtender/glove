#pragma once

#include <Arduino.h>
#include <string>
#include <NimBLEDevice.h>

// BLE gesture framing protocol (all values little-endian):
//   GESTURE_START  0x01  [count_lo][count_hi]                      3 bytes
//   GESTURE_SAMPLE 0x02  [n_samples][n_samples × 13-byte payload]  2+n*13 bytes
//   GESTURE_END    0x03                                             1 byte
//
// 13-byte GloveSample layout:
//   byte  0:     flex_bits  — 5-bit bitmask, bit i = finger i flexed (thumb=0, pinky=4)
//   bytes 1–6:   accel[3]   — ax, ay, az  (int16_t, little-endian)
//   bytes 7–12:  gyro[3]    — gx, gy, gz  (int16_t, little-endian)
static constexpr uint8_t BLE_PKT_GESTURE_START  = 0x01;
static constexpr uint8_t BLE_PKT_GESTURE_SAMPLE = 0x02;
static constexpr uint8_t BLE_PKT_GESTURE_END    = 0x03;
static constexpr size_t  BLE_SAMPLE_BYTES       = 13;
// Max samples packed into one GESTURE_SAMPLE notify:
//   payload = MTU(247) - ATT_overhead(3) - header(type+count=2) = 242 bytes → 18 samples
static constexpr size_t  BLE_SAMPLES_PER_PKT    = (247 - 3 - 2) / BLE_SAMPLE_BYTES;

class BleServer
{
public:
  BleServer() = default;

  void begin();
  bool isConnected() const { return _connected; }

  bool sendGestureWindow(const uint8_t *data, size_t totalSamples);

  bool sendGestureStart(uint16_t sampleCount);
  bool sendGestureSample(const uint8_t *data, size_t n);  
  bool sendGestureEnd();

  bool send(const std::string &msg);
  bool sendRaw(const uint8_t *data, size_t len);

private:
  friend class BleServerCallbacks;

  void _onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc);
  void _onDisconnect();

  NimBLECharacteristic *_txChar    = nullptr;
  bool                   _connected = false;
};
