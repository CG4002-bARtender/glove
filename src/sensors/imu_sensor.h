#pragma once

#include "sensor.h"
#include "../config.h"
#include <MPU6050.h>

class ImuSensor : public Sensor
{
public:
  ImuSensor();

  void setup() override;
  void read() override;
  void print() override;
private:
  MPU6050 mpu;
  int16_t values[6]; // ax, ay, az, gx, gy, gz
};
