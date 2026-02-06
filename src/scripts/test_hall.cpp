#include <Arduino.h>
#include "../config.h"
#include "../sensors/hall_sensor.h"

HallSensor g_hallSensor;

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== Hall Sensor Test Script ===");
  g_hallSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (g_hallSensor.shouldRead(now))
  {
    g_hallSensor.read();
    g_hallSensor.print();
  }
}
