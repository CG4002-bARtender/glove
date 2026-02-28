#include <Arduino.h>
#include "../config.h"

constexpr int INBUILT_LED_PIN = 2;
constexpr int BLINK_DELAY_MS  = 500;

void setup()
{
  DEBUG_INIT();
  pinMode(INBUILT_LED_PIN, OUTPUT);
}

void loop()
{
  DEBUG_PRINTLN("ON");
  digitalWrite(INBUILT_LED_PIN, HIGH);
  delay(BLINK_DELAY_MS);

  DEBUG_PRINTLN("OFF");
  digitalWrite(INBUILT_LED_PIN, LOW);
  delay(BLINK_DELAY_MS);
}
