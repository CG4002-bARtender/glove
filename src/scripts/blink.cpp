#include <Arduino.h>

// DFRobot FireBeetle2 ESP32-E onboard LED is on GPIO 2
constexpr int kLedPin = 2;
constexpr int kBlinkDelayMs = 500;

void setup()
{
  pinMode(kLedPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Blink script running...");
}

void loop()
{
  digitalWrite(kLedPin, HIGH);
  Serial.println("LED ON");
  delay(kBlinkDelayMs);

  digitalWrite(kLedPin, LOW);
  Serial.println("LED OFF");
  delay(kBlinkDelayMs);
}
