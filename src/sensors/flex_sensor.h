#pragma once

#include "sensor.h"
#include "../config.h"

struct FlexData
{
  int flex_values[config::kNumFlexSensors];
};

class FlexSensor : public Sensor
{
public:
  FlexSensor();

  void setup() override;
  void read() override;

  const FlexData& getData() const { return m_data; }

private:
  FlexData m_data;
};
