#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"

FlexSensor g_flexSensor;

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== Flex Sensor Test Script ===");
  g_flexSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (g_flexSensor.shouldRead(now))
  {
    g_flexSensor.read();
  }
}
