#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"

FlexSensor g_flexSensor;

constexpr float kFlexThreshold = 0.15f; // 15% drop = flexed

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== Flex Sensor Test Script ===");
  DEBUG_PRINTLN("Calibrating... keep fingers OPEN for 5 seconds");
  g_flexSensor.setup();
  g_flexSensor.calibrate();

  DEBUG_PRINTLN("=== Calibration Complete ===");
  const float* baseline = g_flexSensor.getBaseline();
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
    Serial.printf("  Finger %d baseline: %.1f\n", i, baseline[i]);
  DEBUG_PRINTLN("============================");
}

void loop()
{
  unsigned long now = millis();

  if (!g_flexSensor.shouldRead(now))
    return;

  g_flexSensor.read();
  const int* data = g_flexSensor.getData();
  const float* baseline = g_flexSensor.getBaseline();

  Serial.printf("=========== FLEX SENSORS ===========\n");
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
  {
    float threshold = baseline[i] * (1.0f - kFlexThreshold);
    bool isFlexed = data[i] < threshold;
    Serial.printf("  Finger %d: %4d  |  isFlexed: %s\n",
                  i, data[i], isFlexed ? "YES" : "NO");
  }
  Serial.printf("=====================================\n");
}
