#include <Arduino.h>
#include "../config.h"
#include "../sensors/imu_sensor.h"

ImuSensor g_imuSensor;

void setup()
{
  DEBUG_INIT();
  delay(1000);
  DEBUG_PRINTLN("=== IMU Sensor Test Script ===");

  g_imuSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (g_imuSensor.shouldRead(now))
  {
    g_imuSensor.read();
    g_imuSensor.print();
  }
}
