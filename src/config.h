#pragma once

#include <Arduino.h>

// Set to 0 to disable all debug serial prints for production
#define DEBUG_MODE 1

#if DEBUG_MODE
  #define DEBUG_INIT()        Serial.begin(config::serial::BAUD_RATE)
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_INIT()        ((void)0)
  #define DEBUG_PRINT(...)    ((void)0)
  #define DEBUG_PRINTLN(...)  ((void)0)
  #define DEBUG_PRINTF(...)   ((void)0)
#endif

namespace config
{
  namespace serial {
    constexpr int         BAUD_RATE = 115200;
  }

  namespace imu {
    constexpr int         SDA_PIN = 21;     
    constexpr int         SCL_PIN = 22;     
    constexpr int         CLOCK_SPEED = 100000;
    constexpr int         INTERVAL_MS = 33;
  }

  namespace flex {
    constexpr int         PINS[] = {A0,A1,A2,A3,A4};
    constexpr size_t      PINS_LEN = sizeof(PINS) / sizeof(int);
    constexpr int         CALIBRATION_ROUNDS = 30;
    constexpr int         CALIBRATION_DELAY = 100;
    constexpr int         INTERVAL_MS = 330;
  }
}
