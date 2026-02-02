#pragma once

#include <Arduino.h>

class Sensor
{
public:
  Sensor(unsigned long intervalMs) : m_intervalMs(intervalMs), m_lastReadMs(0) {}
  virtual ~Sensor() = default;

  virtual void setup() = 0;
  virtual void read() = 0;

  bool shouldRead(unsigned long now)
  {
    if (now - m_lastReadMs >= m_intervalMs)
    {
      m_lastReadMs = now;
      return true;
    }
    return false;
  }

protected:
  unsigned long m_intervalMs;
  unsigned long m_lastReadMs;
};
