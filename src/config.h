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

  namespace buzzer {
    constexpr int         PIN        = 13;
    constexpr int         LEDC_CH    = 0;
    constexpr int         LEDC_RES   = 8;    // bits
  }

  namespace rgb {
    constexpr int         PIN_R      = 25;
    constexpr int         PIN_G      = 26;
    constexpr int         PIN_B      = 27;
    constexpr int         LEDC_CH_R  = 1;
    constexpr int         LEDC_CH_G  = 2;
    constexpr int         LEDC_CH_B  = 3;
    constexpr int         LEDC_FREQ  = 1000; // Hz
    constexpr int         LEDC_RES   = 8;    // bits (0–255)
  }

  namespace ble {
    constexpr const char* DEVICE_NAME       = "ESP32-Glove";
    constexpr const char* SERVICE_UUID      = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    constexpr const char* CHAR_UUID_TX      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
    constexpr const int   PING_INTERVAL_MS  = 1000;
    constexpr const int   PUBLISH_INTERVAL_MS = 50;  // 20 Hz BLE publish rate
    constexpr uint16_t    CONN_MIN_INTERVAL = 16;   // 20 ms (units of 1.25 ms)
    constexpr uint16_t    CONN_MAX_INTERVAL = 32;   // 40 ms
    constexpr uint16_t    CONN_LATENCY      = 0;
    constexpr uint16_t    CONN_TIMEOUT      = 600;  // 6 s  (units of 10 ms)
  }
}
