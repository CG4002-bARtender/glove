#include "activity_detector.h"
#include <math.h>
#include <string.h>

// Physical scale factors (MPU6050 default ranges)
static constexpr float ACCEL_SCALE = (1.0f / 16384.0f) * config::detection::GRAVITY; // LSB → m/s²
static constexpr float GYRO_SCALE  = ((float)M_PI / 180.0f) / 131.0f;               // LSB → rad/s

// ── Constructor ──────────────────────────────────────────────────────────────

ActivityDetector::ActivityDetector()
{
  memset(_window,     0, sizeof(_window));
  memset(_preBuf,     0, sizeof(_preBuf));
  memset(_gestureBuf, 0, sizeof(_gestureBuf));
}

// ── Private helpers ──────────────────────────────────────────────────────────

float ActivityDetector::computeWindowVariance() const
{
  if (_winCount < 2) return 0.0f;

  float mean = 0.0f;
  for (size_t i = 0; i < _winCount; ++i) mean += _window[i];
  mean /= (float)_winCount;

  float var = 0.0f;
  for (size_t i = 0; i < _winCount; ++i) {
    float d = _window[i] - mean;
    var += d * d;
  }
  return var / (float)_winCount;
}

void ActivityDetector::pushPreBuf(const GloveSample& s)
{
  _preBuf[_preBufHead] = s;
  _preBufHead = (_preBufHead + 1) % config::detection::PRE_BUFFER_SIZE;
  if (_preBufCount < config::detection::PRE_BUFFER_SIZE) ++_preBufCount;
}

void ActivityDetector::copyPreBufToGesture()
{
  _gestureLen = 0;
  // Oldest entry sits at (head - count) mod SIZE; walk forward chronologically.
  const size_t start = (_preBufHead + config::detection::PRE_BUFFER_SIZE - _preBufCount)
                       % config::detection::PRE_BUFFER_SIZE;
  for (size_t i = 0; i < _preBufCount; ++i)
    appendGesture(_preBuf[(start + i) % config::detection::PRE_BUFFER_SIZE]);
}

void ActivityDetector::resetPreBuf()
{
  _preBufHead  = 0;
  _preBufCount = 0;
}

void ActivityDetector::appendGesture(const GloveSample& s)
{
  if (_gestureLen < config::detection::MAX_GESTURE_SAMPLES)
    _gestureBuf[_gestureLen++] = s;
}

// ── Score computation ────────────────────────────────────────────────────────

float ActivityDetector::computeActivityScore(const GloveSample& s)
{
  // 1. Dynamic acceleration (gravity-subtracted magnitude)
  const float ax = s.accel[0] * ACCEL_SCALE;
  const float ay = s.accel[1] * ACCEL_SCALE;
  const float az = s.accel[2] * ACCEL_SCALE;
  const float accelMag     = sqrtf(ax*ax + ay*ay + az*az);
  const float dynamicAccel = fabsf(accelMag - config::detection::GRAVITY);

  // Push into sliding window and compute variance
  _window[_winHead] = dynamicAccel;
  _winHead = (_winHead + 1) % config::detection::WINDOW_SIZE;
  if (_winCount < config::detection::WINDOW_SIZE) ++_winCount;

  const float accelVariance = computeWindowVariance();

  // 2. Gyroscope magnitude (rad/s)
  const float gx = s.gyro[0] * GYRO_SCALE;
  const float gy = s.gyro[1] * GYRO_SCALE;
  const float gz = s.gyro[2] * GYRO_SCALE;
  const float gyroMag = sqrtf(gx*gx + gy*gy + gz*gz);

  // 3. Flex rate-of-change (count of bits that toggled since last sample)
  float flexActivity = 0.0f;
  if (_hasPrevFlex) {
    flexActivity = (float)__builtin_popcount(s.flex_bits ^ _prevFlexBits)
                 * config::detection::FLEX_SCALE;
  }
  _prevFlexBits = s.flex_bits;
  _hasPrevFlex  = true;

  // 4. Weighted composite → EMA
  const float raw = config::detection::W1_ACCEL * accelVariance
                  + config::detection::W2_GYRO  * gyroMag
                  + config::detection::W3_FLEX  * flexActivity;

  _emaScore = config::detection::EMA_ALPHA * raw
            + (1.0f - config::detection::EMA_ALPHA) * _emaScore;

  return _emaScore;
}

// ── State machine ─────────────────────────────────────────────────────────────

bool ActivityDetector::update(const GloveSample& s)
{
  const float score = computeActivityScore(s);

  switch (_state)
  {
  case State::IDLE:
    // Always keep the pre-buffer fresh so gesture onset is preserved.
    pushPreBuf(s);

    if (score >= config::detection::THRESH_HIGH) {
      if (++_activeCount >= config::detection::MIN_ACTIVE_SAMPLES) {
        // IDLE → ACTIVE: pre-buffer already contains the onset samples.
        copyPreBufToGesture();
        _state       = State::ACTIVE;
        _idleCount   = 0;
        _activeCount = 0;
        DEBUG_PRINTF("[Detect] ACTIVE  score=%.3f\n", score);
      }
    } else {
      _activeCount = 0;
    }
    break;

  case State::ACTIVE:
    appendGesture(s);

    if (score < config::detection::THRESH_LOW) {
      if (++_idleCount >= config::detection::IDLE_TIMEOUT_SAMPLES) {
        // ACTIVE → IDLE: gesture window is complete.
        _state       = State::IDLE;
        _activeCount = 0;
        _idleCount   = 0;
        resetPreBuf();
        DEBUG_PRINTF("[Detect] IDLE    gesture=%u samples\n", _gestureLen);
        return true;  // caller may now read gestureData() / gestureLength()
      }
    } else {
      _idleCount = 0;
    }
    break;
  }

  return false;
}
