#pragma once

#include "sensor.h"
#include "../config.h"
#include <MPU6050.h>

struct ImuData
{
  int16_t accel[3];  // x, y, z
  int16_t gyro[3];   // x, y, z
};

class ImuSensor : public Sensor
{
public:
  ImuSensor();

  void setup() override;
  void read() override;

  const ImuData& getData() const { return m_data; }
  bool isConnected() const { return m_connected; }

private:
  MPU6050 m_mpu;
  ImuData m_data;
  bool m_connected;
};
