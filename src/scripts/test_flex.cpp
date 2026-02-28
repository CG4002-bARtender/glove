#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"

FlexSensor flexSensor;

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== Flex Sensor Test Script ===");
  flexSensor.setup();
  flexSensor.calibrate();
}

void loop()
{
  unsigned long now = millis();

  if (flexSensor.shouldRead(now)) {
    flexSensor.read();
    flexSensor.print();
  }
}
