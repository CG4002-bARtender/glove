#pragma once

#include <Arduino.h>

class Actuator
{
public:
  virtual ~Actuator() = default;

  virtual void setup() = 0;
  virtual void print() = 0;
};
