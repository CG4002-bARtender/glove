#pragma once

#include "sensor.h"
#include "../config.h"

class HallSensor : public Sensor
{
public:
  HallSensor();

  void setup() override;
  void read() override;
  void print() override;

private:
  void calibrate();

  int offset_values[config::kNumHallSensors];
  int baseline_values[config::kNumHallSensors];
};
