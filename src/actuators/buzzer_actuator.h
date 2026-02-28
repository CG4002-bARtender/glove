#pragma once

#include "actuator.h"
#include "../config.h"

class BuzzerActuator : public Actuator
{
public:
  BuzzerActuator();

  void setup() override;
  void print() override;

  void tone(uint16_t freq);
  void silence();

private:
  uint16_t _freq;
};
