#include <Arduino.h>
#include "../config.h"
#include "../sensors/imu_sensor.h"

ImuSensor g_imuSensor;

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== IMU Test Script ===");
  Serial.println("Initializing MPU6050...");

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
