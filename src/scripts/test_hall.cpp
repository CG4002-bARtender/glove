#include <Arduino.h>
#include "../config.h"
#include "../sensors/hall_sensor.h"

HallSensor g_hallSensor;

static constexpr int kCalibrationSamples = 30;
static int g_baseline[config::kNumHallSensors] = {};

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== Hall Sensor Test Script ===");
  g_hallSensor.setup();

  // Calibration: collect 30 readings per sensor and average
  Serial.println("Calibrating... keep magnets away!");
  long sums[config::kNumHallSensors] = {};
  for (int s = 0; s < kCalibrationSamples; ++s)
  {
    for (size_t i = 0; i < config::kNumHallSensors; ++i)
    {
      sums[i] += analogRead(config::kHallPins[i]);
    }
    delay(50);
  }
  for (size_t i = 0; i < config::kNumHallSensors; ++i)
  {
    g_baseline[i] = sums[i] / kCalibrationSamples;
    Serial.printf("[Hall A%d] baseline: %d\n", i, g_baseline[i]);
  }
  Serial.println("Calibration done.\n");
}

void loop()
{
  unsigned long now = millis();

  if (g_hallSensor.shouldRead(now))
  {
    g_hallSensor.read();
    const HallData& data = g_hallSensor.getData();

    Serial.printf("=========== Hall SENSORS ===========\n");
    int maxAbs = 0;
    int closestIdx = -1;
    for (size_t i = 0; i < config::kNumHallSensors; ++i)
    {
      int offset = data.Hall[i] - g_baseline[i];
      Serial.printf("[Hall A%d]: %+d  (raw: %d)\n", i, offset, data.Hall[i]);
      if (abs(offset) > maxAbs)
      {
        maxAbs = abs(offset);
        closestIdx = i;
      }
    }
    constexpr int kNoiseThreshold = 20;
    if (maxAbs > kNoiseThreshold)
      Serial.printf(">> Magnet closest to: A%d  (|offset| = %d)\n", closestIdx, maxAbs);
    else
      Serial.printf(">> No magnet detected\n");
    Serial.printf("=====================================\n");
  }
}
