#pragma once

#include <Arduino.h>
#include "hand_state.h"
#include "../config.h"

class StateDetector
{
public:
  StateDetector();

  // Feed latest sensor readings. Returns true if state transitioned.
  bool update(const int* flex, const int16_t* imu);

  HandState current() const { return _current; }

private:
  HandState classify(const int* flex, const int16_t* imu);
  bool      isShaking(const int16_t* accel);

  // Rolling |a|² buffer for shake variance
  int64_t _magSq[config::state::SHAKE_WINDOW] = {};
  int     _magSqHead  = 0;
  int     _magSqCount = 0;

  // Per-finger hysteresis state
  bool    _indexFlex  = false;
  bool    _middleFlex = false;
  bool    _ringFlex   = false;


  // Debounce
  HandState _current   = HandState::RELEASE;
  HandState _candidate = HandState::RELEASE;
  int       _debounce  = 0;
};
