#pragma once

#include <Arduino.h>
#include "../config.h"

struct __attribute__((packed)) GloveSample {
  uint8_t  flex_bits;  // 5-bit bitmask: bit 0 = thumb, bit 4 = pinky
  int16_t  accel[3];   // ax, ay, az  (MPU6050 raw, ±2 g)
  int16_t  gyro[3];    // gx, gy, gz  (MPU6050 raw, ±250 °/s)
};

class ActivityDetector
{
public:
  enum class State { IDLE, ACTIVE };

  ActivityDetector();

  bool update(const GloveSample& s);

  State  getState()      const { return _state; }

  const GloveSample* gestureData()   const { return _gestureBuf; }
  size_t             gestureLength() const { return _gestureLen; }

private:
  float computeActivityScore(const GloveSample& s);

  // Sliding window ring buffer 
  float  _window[config::detection::WINDOW_SIZE];
  size_t _winHead  = 0;
  size_t _winCount = 0;
  float  computeWindowVariance() const;

  float _emaScore = 0.0f;

  uint8_t _prevFlexBits = 0;
  bool    _hasPrevFlex  = false;

  State  _state       = State::IDLE;
  size_t _activeCount = 0; 
  size_t _idleCount   = 0; 

  GloveSample _preBuf[config::detection::PRE_BUFFER_SIZE];
  size_t      _preBufHead  = 0;
  size_t      _preBufCount = 0;

  void pushPreBuf(const GloveSample& s);
  void copyPreBufToGesture();
  void resetPreBuf();

  GloveSample _gestureBuf[config::detection::MAX_GESTURE_SAMPLES];
  size_t      _gestureLen = 0;

  void appendGesture(const GloveSample& s);
};
