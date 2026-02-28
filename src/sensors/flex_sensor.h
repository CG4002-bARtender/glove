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
  int readings[config::flex::PINS_LEN];
  int baseline[config::flex::PINS_LEN];
};
