#include "flex_sensor.h"

FlexSensor::FlexSensor() : Sensor(config::FLEX_INTERVAL_MS), readings{}, baseline{}, calibrated(false) {}

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
    readings[i] = analogRead(config::FLEX_SENSOR_PINS[i]);
  }
}

void FlexSensor::print()
{
  Serial.printf("=========== FLEX SENSORS ===========\n");
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
  {
    Serial.printf("[Flex Pin: A%d]: %d\n", i, readings[i]);
  }
  Serial.printf("=====================================\n");
}

void FlexSensor::calibrate(unsigned long durationMs)
{
  long baselineSum[config::FLEX_SENSOR_PINS_LEN] = {};
  int count = 0;
  unsigned long start = millis();

  while (millis() - start < durationMs)
  {
    read();
    for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
      baselineSum[i] += readings[i];
    count++;
    delay(intervalMs);
  }

  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
    baseline[i] = (float)baselineSum[i] / count;

  calibrated = true;
}

const int* FlexSensor::getData() const
{
  return readings;
}

const float* FlexSensor::getBaseline() const
{
  return baseline;
}

bool FlexSensor::isCalibrated() const
{
  return calibrated;
}
