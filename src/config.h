#pragma once

#include <Arduino.h>

namespace config
{
  // Serial
  constexpr int kBaudRate = 9600;

  // Flex sensor config
  constexpr int kFlexPins[] = {A0, A1, A2, A3, A4};
  constexpr size_t kNumFlexSensors = sizeof(kFlexPins) / sizeof(kFlexPins[0]);
  constexpr unsigned long kFlexIntervalMs = 1000;
  constexpr int kFlexThreshold = 3600;

  // IMU config
  constexpr unsigned long kImuIntervalMs = 20;  // 50Hz
}
