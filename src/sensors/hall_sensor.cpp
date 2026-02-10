#include "hall_sensor.h"

HallSensor::HallSensor() : Sensor(config::kHallIntervalMs) {};

void HallSensor::setup()
{
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    pinMode(config::kHallPins[i], INPUT);
    analogSetPinAttenuation(config::kHallPins[i], ADC_11db);
  }
  calibrate();
}

void HallSensor::read()
{
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    offset_values[i] = analogRead(config::kHallPins[i]) - baseline_values[i];
  }
}

void HallSensor::print()
{
  Serial.printf("=========== Hall SENSORS ===========\n");
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    Serial.printf("[Hall A%d]: %+d\n", i, offset_values[i]);
  }
  Serial.printf("=====================================\n");
}

void HallSensor::calibrate() 
{
  for (size_t i = 0; i < 30; ++i) 
  {
    for (size_t j = 0; j < config::kNumHallSensors; ++j)
    {
      baseline_values[j] += analogRead(config::kHallPins[j]);
    }
    delay(200);
  }

  for (size_t j = 0; j < config::kNumHallSensors; ++j)
  {
    baseline_values[j] = baseline_values[j] / 30; 
  }
}
