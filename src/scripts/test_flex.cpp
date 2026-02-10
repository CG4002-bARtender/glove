#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"

FlexSensor g_flexSensor;

constexpr unsigned long kCalibrationMs = 5000;
constexpr float kFlexThreshold = 0.15f; // 15% drop = flexed

float g_baseline[config::kNumFlexSensors] = {};
long g_baselineSum[config::kNumFlexSensors] = {};
int g_baselineCount = 0;
bool g_calibrated = false;

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== Flex Sensor Test Script ===");
  Serial.println("Calibrating... keep fingers OPEN for 5 seconds");
  g_flexSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (!g_flexSensor.shouldRead(now))
    return;

  g_flexSensor.read();
  const FlexData& data = g_flexSensor.getData();

  if (!g_calibrated)
  {
    for (size_t i = 0; i < config::kNumFlexSensors; ++i)
      g_baselineSum[i] += data.flex[i];
    g_baselineCount++;

    Serial.printf("Calibrating... %lus remaining\n",
                  (kCalibrationMs - min(now, kCalibrationMs)) / 1000);

    if (now >= kCalibrationMs)
    {
      for (size_t i = 0; i < config::kNumFlexSensors; ++i)
        g_baseline[i] = (float)g_baselineSum[i] / g_baselineCount;

      g_calibrated = true;
      Serial.println("=== Calibration Complete ===");
      for (size_t i = 0; i < config::kNumFlexSensors; ++i)
        Serial.printf("  Finger %d baseline: %.1f\n", i, g_baseline[i]);
      Serial.println("============================");
    }
    return;
  }

  // Post-calibration: print raw + isFlexed
  Serial.printf("=========== FLEX SENSORS ===========\n");
  for (size_t i = 0; i < config::kNumFlexSensors; ++i)
  {
    float threshold = g_baseline[i] * (1.0f - kFlexThreshold);
    bool isFlexed = data.flex[i] < threshold;
    Serial.printf("  Finger %d: %4d  |  isFlexed: %s\n",
                  i, data.flex[i], isFlexed ? "YES" : "NO");
  }
  Serial.printf("=====================================\n");
}
