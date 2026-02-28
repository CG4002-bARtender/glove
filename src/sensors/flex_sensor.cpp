#include "flex_sensor.h"

FlexSensor::FlexSensor() : Sensor(config::flex::INTERVAL_MS), readings{}, baseline{} {}

void FlexSensor::setup()
{
  for (size_t i = 0; i < config::flex::PINS_LEN; ++i)
  {
    pinMode(config::flex::PINS[i], INPUT);
  }
}

void FlexSensor::read()
{
  for (size_t i = 0; i < config::flex::PINS_LEN; ++i)
  {
    readings[i] = analogRead(config::flex::PINS[i]) - baseline[i];
  }
}

void FlexSensor::print()
{
  DEBUG_PRINTLN("=========== FLEX SENSORS ==========");
  for (size_t i = 0; i < config::flex::PINS_LEN; ++i)
  {
    DEBUG_PRINTF("[Flex Pin: A%d]: %d\n", i, readings[i]);
  }
  DEBUG_PRINTLN("=====================================");
}

void FlexSensor::calibrate()
{
  for (size_t i = 0; i < config::flex::CALIBRATION_ROUNDS; ++i) {
    for (size_t j = 0; j < config::flex::PINS_LEN; ++j) {
      baseline[j] += analogRead(config::flex::PINS[j]);
    }
    delay(config::flex::CALIBRATION_DELAY);
  } 

  for (size_t j = 0; j < config::flex::PINS_LEN; ++j) {
      baseline[j] /= config::flex::CALIBRATION_ROUNDS;
  }
}
