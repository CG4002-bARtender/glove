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

  void calibrate();
private:
  int readings[config::FLEX_SENSOR_PINS_LEN];
  int baseline[config::FLEX_SENSOR_PINS_LEN];
};
