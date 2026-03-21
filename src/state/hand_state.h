#pragma once

#include <Arduino.h>

enum class HandState : uint8_t
{
  GRAB    = 0,
  RELEASE = 1,
  POUR    = 2,
  SHAKE   = 3,
  SERVE   = 4,
};

inline const char* handStateName(HandState s)
{
  switch (s)
  {
    case HandState::GRAB:    return "GRAB";
    case HandState::RELEASE: return "RELEASE";
    case HandState::POUR:    return "POUR";
    case HandState::SHAKE:   return "SHAKE";
    case HandState::SERVE:   return "SERVE";
    default:                 return "UNKNOWN";
  }
}
