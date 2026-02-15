#include <Arduino.h>
#include "../config.h"

void setup()
{
  DEBUG_INIT();
  pinMode(config::INBUILT_LED_PIN, OUTPUT);
}

void loop()
{
  Serial.print("ON");
  digitalWrite(config::INBUILT_LED_PIN, HIGH);
  delay(config::BLINK_DELAY_MS);

  Serial.println("OFF");
  digitalWrite(config::INBUILT_LED_PIN, LOW);
  delay(config::BLINK_DELAY_MS);
}
