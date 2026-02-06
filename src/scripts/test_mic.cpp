#include <driver/i2s.h>
#include <Arduino.h>
#include <config.h>
#include "../sensors/mic_sensor.h"

MicSensor g_micSensor;

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("\n\n=== INMP441 Connection Test ===\n");

  g_micSensor.setup();
}

void loop()
{
  unsigned long now = millis();

  if (g_micSensor.shouldRead(now))
  {
    g_micSensor.read();
    g_micSensor.print();
  }
}