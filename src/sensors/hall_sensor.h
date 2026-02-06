#pragma once

#include "sensor.h"
#include "../config.h"

struct HallData
{
  int Hall[config::kNumHallSensors];
};

class HallSensor : public Sensor
{
public:
  HallSensor();

  void setup() override;
  void read() override;
  void print() override;

  const HallData& getData() const { return m_data; }

private:
  HallData m_data;
};
