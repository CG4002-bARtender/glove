#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"
#include "../sensors/imu_sensor.h"

FlexSensor flexSensor;
ImuSensor imuSensor;

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== Flex Sensor Test Script ===");
  flexSensor.setup();
  flexSensor.calibrate();

  imuSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (flexSensor.shouldRead(now)) {
    flexSensor.read();
    flexSensor.print();
  }

  if (imuSensor.shouldRead(now))
  {
    imuSensor.read();
    imuSensor.print();
  }
}
