#pragma once

#include <driver/i2s.h>

#include "sensor.h"
#include "../config.h"

class MicSensor : public Sensor
{
public:
  MicSensor();
  ~MicSensor();

  void setup() override;
  void read() override;
  void print() override;

private:
  int32_t samples[64];
  size_t bytes_read;
};
