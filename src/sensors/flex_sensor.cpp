#include "flex_sensor.h"

FlexSensor::FlexSensor() : Sensor(config::kFlexIntervalMs), m_data{} {}

void FlexSensor::setup()
{
  for (size_t i = 0; i < config::kNumFlexSensors; ++i)
  {
    pinMode(config::kFlexPins[i], INPUT);
  }
}

void FlexSensor::read()
{
  for (size_t i = 0; i < config::kNumFlexSensors; ++i)
  {
    m_data.flex[i] = analogRead(config::kFlexPins[i]);
  }
}

void FlexSensor::print()
{
  Serial.printf("=========== FLEX SENSORS ===========\n");
  for (size_t i = 0; i < config::kNumFlexSensors; ++i)
  {
    Serial.printf("[Flex Pin: A%d]: %d\n", i, m_data.flex[i]);
  }
  Serial.printf("=====================================\n");
}
