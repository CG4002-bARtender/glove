#include <Arduino.h>
#include "../config.h"
#include "../sensors/imu_sensor.h"

ImuSensor imuSensor;

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== IMU Sensor Test Script ===");
  imuSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (imuSensor.shouldRead(now))
  {
    imuSensor.read();
    imuSensor.print();
  }
}
