#include <Arduino.h>
#include "../config.h"

void setup()
{
  Serial.begin(config::kBaudRate);
  pinMode(config::kLedPin, OUTPUT);
  pinMode(config::kRecordButtonPin, INPUT_PULLUP);
  Serial.println("=== Button Test ===");
  Serial.printf("Button pin: GPIO %d (INPUT_PULLUP)\n", config::kRecordButtonPin);
  Serial.println("Press button to toggle LED...");
}

void loop()
{
  bool pressed = !digitalRead(config::kRecordButtonPin);
  digitalWrite(config::kLedPin, pressed ? HIGH : LOW);

  if (pressed)
  {
    Serial.println("PRESSED");
    delay(200);
  }
}
