#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"
#include "../sensors/imu_sensor.h"
#include "../comms/ble_server.h"

// 22-byte binary packet: 5 flex + 3 accel + 3 gyro, all int16_t, little-endian
struct __attribute__((packed)) GlovePacket {
  int16_t flex[5];   // baseline-subtracted flex deltas, fingers 0-4
  int16_t accel[3];  // ax, ay, az
  int16_t gyro[3];   // gx, gy, gz
};

static FlexSensor flex;
static ImuSensor  imu;
static BleServer  ble;

static unsigned long lastPublishMs = 0;

void setup()
{
  DEBUG_INIT();

  ble.begin();
  imu.setup();

  DEBUG_PRINTLN("Calibrating flex sensors, hold hand flat...");
  flex.setup();
  flex.calibrate();
  DEBUG_PRINTLN("Calibration done. Ready.");
}

void loop()
{
  unsigned long now = millis();

  if (imu.shouldRead(now)) imu.read();
  if (flex.shouldRead(now)) flex.read();

  if (ble.isConnected() && ble.shouldPublish(now))
  {
    const int*     f = flex.getData();  // int, baseline-subtracted
    const int16_t* d = imu.getData();   // ax, ay, az, gx, gy, gz

    GlovePacket pkt;
    for (int i = 0; i < 5; ++i) pkt.flex[i]  = static_cast<int16_t>(f[i]);
    for (int i = 0; i < 3; ++i) pkt.accel[i] = d[i];
    for (int i = 0; i < 3; ++i) pkt.gyro[i]  = d[3 + i];

    ble.sendRaw(reinterpret_cast<const uint8_t*>(&pkt), sizeof(pkt));
  }
}
