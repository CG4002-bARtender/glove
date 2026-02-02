#include <Arduino.h>
#include "../config.h"

void setup()
{
  pinMode(config::kLedPin, OUTPUT);
  Serial.begin(config::kBaudRate);
  Serial.println("=== Blink Test Script ===");
}

void loop()
{
  digitalWrite(config::kLedPin, HIGH);
  Serial.println("LED ON");
  delay(config::kBlinkDelayMs);

  digitalWrite(config::kLedPin, LOW);
  Serial.println("LED OFF");
  delay(config::kBlinkDelayMs);
}
