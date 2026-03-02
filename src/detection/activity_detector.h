#pragma once

#include <Arduino.h>
#include "../config.h"

// Raw sensor sample — 13 bytes, same binary layout as the BLE GlovePacket.
// flex_bits: bit i = finger i flexed (0=thumb … 4=pinky), threshold in config::flex::THRESHOLDS
struct __attribute__((packed)) GloveSample {
  uint8_t  flex_bits;  // 5-bit bitmask: bit 0 = thumb, bit 4 = pinky
  int16_t  accel[3];   // ax, ay, az  (MPU6050 raw, ±2 g)
  int16_t  gyro[3];    // gx, gy, gz  (MPU6050 raw, ±250 °/s)
};
static_assert(sizeof(GloveSample) == 13, "GloveSample must be 13 bytes");

// ---------------------------------------------------------------------------
// ActivityDetector
//
// Runs a lightweight, math-only activity gate on every IMU sample (30 Hz).
// While idle, it maintains a rolling pre-buffer so gesture onset is never
// clipped.  When a gesture is detected and then ends, update() returns true
// and gestureData()/gestureLength() expose the complete capture window.
// ---------------------------------------------------------------------------
class ActivityDetector
{
public:
  enum class State { IDLE, ACTIVE };

  ActivityDetector();

  // Feed one sample.  Returns true the moment a gesture window is finalized
  // (ACTIVE → IDLE transition with hysteresis satisfied).
  bool update(const GloveSample& s);

  State  getState()      const { return _state; }

  // Valid only immediately after update() returns true.
  const GloveSample* gestureData()   const { return _gestureBuf; }
  size_t             gestureLength() const { return _gestureLen; }

private:
  // ── Scoring ──────────────────────────────────────────────────────────────
  float computeActivityScore(const GloveSample& s);

  // Sliding window ring buffer (stores dynamic-accel magnitude for variance)
  float  _window[config::detection::WINDOW_SIZE];
  size_t _winHead  = 0;
  size_t _winCount = 0;
  float  computeWindowVariance() const;

  // EMA-smoothed composite score
  float _emaScore = 0.0f;

  // Previous flex bitmask for first-derivative (bit-change count)
  uint8_t _prevFlexBits = 0;
  bool    _hasPrevFlex  = false;

  // ── State machine ─────────────────────────────────────────────────────────
  State  _state       = State::IDLE;
  size_t _activeCount = 0;   // consecutive samples ≥ THRESH_HIGH
  size_t _idleCount   = 0;   // consecutive samples <  THRESH_LOW

  // ── Pre-buffer (ring) ────────────────────────────────────────────────────
  GloveSample _preBuf[config::detection::PRE_BUFFER_SIZE];
  size_t      _preBufHead  = 0;
  size_t      _preBufCount = 0;

  void pushPreBuf(const GloveSample& s);
  void copyPreBufToGesture();
  void resetPreBuf();

  // ── Gesture capture buffer (linear) ─────────────────────────────────────
  GloveSample _gestureBuf[config::detection::MAX_GESTURE_SAMPLES];
  size_t      _gestureLen = 0;

  void appendGesture(const GloveSample& s);
};
