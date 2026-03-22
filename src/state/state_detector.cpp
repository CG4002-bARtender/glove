#include "state_detector.h"

StateDetector::StateDetector() {}

// Returns true if the last SHAKE_WINDOW samples show high accel spread.
// Always updates the rolling buffer so it stays current even in non-grab states.
bool StateDetector::isShaking(const int16_t* accel)
{
  const int64_t ax = accel[0], ay = accel[1], az = accel[2];
  _magSq[_magSqHead] = ax*ax + ay*ay + az*az;
  _magSqHead = (_magSqHead + 1) % config::state::SHAKE_WINDOW;
  if (_magSqCount < config::state::SHAKE_WINDOW) ++_magSqCount;

  if (_magSqCount < config::state::SHAKE_WINDOW) return false;

  int64_t minVal = _magSq[0], maxVal = _magSq[0];
  for (int i = 1; i < config::state::SHAKE_WINDOW; ++i)
  {
    if (_magSq[i] < minVal) minVal = _magSq[i];
    if (_magSq[i] > maxVal) maxVal = _magSq[i];
  }
  return (maxVal - minVal) > config::state::SHAKE_SPREAD_THRESHOLD;
}

HandState StateDetector::classify(const int* flex, const int16_t* imu)
{
  // Update shake buffer every sample regardless of flex state
  const bool shaking = isShaking(imu);

  // Pin mapping: flex[0]=thumb(A0), flex[1]=index(A1), flex[2]=middle(A2),
  //              flex[3]=ring(A3), flex[4]=unused(A4)
  const bool four_curled = flex[1] > config::state::FLEX_INDEX
                        && flex[2] > config::state::FLEX_MIDDLE
                        && flex[3] > config::state::FLEX_RING;

  const bool grab_flex  = four_curled && flex[0] > config::state::FLEX_THUMB_IN;
  const bool serve_flex = four_curled && flex[0] < config::state::FLEX_THUMB_OUT;

  if (grab_flex)
  {
    if (shaking)                                      return HandState::SHAKE;
    if (imu[1] > config::state::POUR_AY_THRESHOLD)   return HandState::POUR;
    return HandState::GRAB;
  }
  if (serve_flex) return HandState::SERVE;
  return HandState::RELEASE;
}

bool StateDetector::update(const int* flex, const int16_t* imu)
{
  const HandState raw = classify(flex, imu);

  // When leaving SHAKE require more consecutive samples to confirm the exit
  const int threshold = (_current == HandState::SHAKE && raw != HandState::SHAKE)
                        ? config::state::SHAKE_EXIT_DEBOUNCE
                        : config::state::DEBOUNCE_SAMPLES;

  if (raw != _candidate)
  {
    _candidate = raw;
    _debounce  = 1;
  }
  else if (_debounce < threshold)
  {
    ++_debounce;
  }

  if (_debounce >= threshold && _candidate != _current)
  {
    _current = _candidate;
    return true;
  }
  return false;
}
