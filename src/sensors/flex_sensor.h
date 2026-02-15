#pragma once

#include "sensor.h"
#include "../config.h"

class FlexSensor : public Sensor
{
public:
  FlexSensor();

  void setup() override;
  void read() override;
  void print() override;

  void calibrate(unsigned long durationMs = 5000);
  const int* getData() const;
  const float* getBaseline() const;
  bool isCalibrated() const;

private:
  int readings[config::FLEX_SENSOR_PINS_LEN];
  float baseline[config::FLEX_SENSOR_PINS_LEN];
  bool calibrated;
};
