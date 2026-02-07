#include <Arduino.h>
#include "../config.h"

void setup()
{
  Serial.begin(config::kBaudRate);
  pinMode(config::kLedPin, OUTPUT);
}

void loop()
{
  Serial.print("ON");
  digitalWrite(config::kLedPin, HIGH);
  delay(config::kBlinkDelayMs);

  Serial.println("OFF");
  digitalWrite(config::kLedPin, LOW);
  delay(config::kBlinkDelayMs);
}
