#include "flex_sensor.h"

FlexSensor::FlexSensor() : Sensor(config::FLEX_INTERVAL_MS), readings{}, baseline{} {}

void FlexSensor::setup()
{
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
  {
    pinMode(config::FLEX_SENSOR_PINS[i], INPUT);
  }
}

void FlexSensor::read()
{
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
  {
    readings[i] = analogRead(config::FLEX_SENSOR_PINS[i]) - baseline[i];
  }
}

void FlexSensor::print()
{
  DEBUG_PRINTLN("=========== FLEX SENSORS ==========");
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
  {
    DEBUG_PRINTF("[Flex Pin: A%d]: %d\n", i, readings[i]);
  }
  DEBUG_PRINTLN("=====================================");
}

void FlexSensor::calibrate()
{
  for (size_t i = 0; i < config::FLEX_SENSOR_CALIBRATION_ROUNDS; ++i) {
    for (size_t j = 0; j < config::FLEX_SENSOR_PINS_LEN; ++j) {
      baseline[i] += analogRead(config::FLEX_SENSOR_PINS[j]);
    }
    delay(config::FLEX_SENSOR_CALIBRATION_DELAY);
  } 

  for (size_t j = 0; j < config::FLEX_SENSOR_PINS_LEN; ++j) {
      baseline[j] /= config::FLEX_SENSOR_CALIBRATION_ROUNDS;
  }
}
