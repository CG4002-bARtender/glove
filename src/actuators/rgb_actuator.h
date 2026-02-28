#pragma once

#include "actuator.h"
#include "../config.h"

class RgbActuator : public Actuator
{
public:
  RgbActuator();

  void setup() override;
  void print() override;

  // Values 0–255 per channel. Assumes common-cathode (high = bright).
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void off();

private:
  uint8_t _r, _g, _b;
};
