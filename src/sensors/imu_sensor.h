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

  int16_t getAx() const { return ax; }
  int16_t getAy() const { return ay; }
  int16_t getAz() const { return az; }
  int16_t getGx() const { return gx; }
  int16_t getGy() const { return gy; }
  int16_t getGz() const { return gz; }

private:
  MPU6050 mpu;
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};
