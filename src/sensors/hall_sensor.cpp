#include "hall_sensor.h"

HallSensor::HallSensor() : Sensor(config::kHallIntervalMs), m_data{} {}

void HallSensor::setup()
{
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    pinMode(config::kHallPins[i], INPUT);
    analogSetPinAttenuation(config::kHallPins[i], ADC_11db);
  }
}

void HallSensor::read()
{
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    m_data.Hall[i] = analogRead(config::kHallPins[i]);
  }
}

void HallSensor::print()
{
  Serial.printf("=========== Hall SENSORS ===========\n");
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    Serial.printf("[Hall Pin: A%d]: %d\n", i, m_data.Hall[i]);
  }
  Serial.printf("=====================================\n");
}
