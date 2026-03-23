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
    constexpr int         PINS[] = {A0,A1,A2,A3};
    constexpr size_t      PINS_LEN = sizeof(PINS) / sizeof(int);
    constexpr int         CALIBRATION_ROUNDS = 30;
    constexpr int         CALIBRATION_DELAY = 100;
    constexpr int         INTERVAL_MS = 330;
    constexpr int         THRESHOLDS[5] = { 300, 500, 400, 50, 100 };
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

  namespace wifi {
    constexpr const char* SSID     = "IphoneAlam";
    constexpr const char* PASSWORD = "pasuhagu";
  }

  namespace mqtt {
    constexpr const char* BROKER              = "172.20.10.2";
    constexpr int         PORT             = 1883;
    constexpr const char* USERNAME         = "test";
    constexpr const char* PASSWORD         = "test";
    constexpr const char* CLIENT_ID        = "glove";
    constexpr const char* TOPIC_STATE      = "glove";
    constexpr size_t      CHUNK_SIZE       = 256;
    constexpr size_t      JSON_BUFFER_SIZE = 128;
  }

  namespace state {
    // Flex thresholds — hysteresis: ENTER > EXIT to prevent boundary oscillation
    constexpr int     FLEX_INDEX_ENTER       = 220;
    constexpr int     FLEX_INDEX_EXIT        = 150;
    constexpr int     FLEX_MIDDLE_ENTER      = 120;
    constexpr int     FLEX_MIDDLE_EXIT       = 70;
    constexpr int     FLEX_RING_ENTER        = 100;
    constexpr int     FLEX_RING_EXIT         = 55;
    constexpr int     FLEX_THUMB_IN          = 200;  // thumb flexed → GRAB
    constexpr int     FLEX_THUMB_OUT         = 0;    // thumb extended → SERVE

    // IMU thresholds (raw MPU6050 int16_t, ±2 g scale)
    constexpr int     POUR_AY_THRESHOLD      = 2500; // ay > 2500 → wrist tilted to pour
    constexpr int64_t SHAKE_SPREAD_THRESHOLD = 200000000LL; // max-min of |a|² window

    // Debounce: consecutive samples required before state transition (~100 ms at 30 Hz)
    constexpr int     DEBOUNCE_SAMPLES       = 10;
    // How many consecutive non-SHAKE samples needed to exit SHAKE (~500 ms at 30 Hz)
    constexpr int     SHAKE_EXIT_DEBOUNCE    = 15;
    constexpr int     SHAKE_WINDOW           = 6;    // rolling |a|² buffer depth (~200 ms)
  }

  namespace detection {
    // Sliding window for accel variance (~200 ms at 30 Hz)
    constexpr size_t  WINDOW_SIZE          = 6;

    // Pre-buffer: raw samples kept during IDLE for gesture-onset capture (~167 ms)
    constexpr size_t  PRE_BUFFER_SIZE      = 5;

    // Hard cap on gesture window length (4 s max at 30 Hz)
    constexpr size_t  MAX_GESTURE_SAMPLES  = 120;

    // Hysteresis thresholds for the activity score (tune empirically)
    constexpr float   THRESH_HIGH          = 1.5f;
    constexpr float   THRESH_LOW           = 0.9f;

    // Debounce: consecutive samples required to confirm state change
    constexpr size_t  MIN_ACTIVE_SAMPLES   = 2;    // ~67 ms at 30 Hz
    constexpr size_t  IDLE_TIMEOUT_SAMPLES = 9;    // ~300 ms at 30 Hz

    // Composite-score weights (tune to balance sensor contribution)
    constexpr float   W1_ACCEL             = 1.0f;
    constexpr float   W2_GYRO              = 1.0f;
    constexpr float   W3_FLEX              = 1.5f;

    // EMA smoothing factor (lower = smoother / more lag)
    constexpr float   EMA_ALPHA            = 0.2f;

    // Physical constants
    constexpr float   GRAVITY              = 9.81f; // m/s²
    // Flex activity scale: flex_bits are binary, max 5 bit-changes per sample.
    constexpr float   FLEX_SCALE           = 1.0f / 5.0f;
  }
}
