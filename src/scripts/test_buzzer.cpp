#include <Arduino.h>
#include "../config.h"
#include "../actuators/buzzer_actuator.h"

BuzzerActuator buzzer;

// Simple scale: C4 D4 E4 F4 G4 A4 B4 C5
static const uint16_t SCALE[] = { 262, 294, 330, 349, 392, 440, 494, 523 };
static const size_t   SCALE_LEN = sizeof(SCALE) / sizeof(uint16_t);

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== Buzzer Actuator Test Script ===");
  buzzer.setup();
}

void loop()
{
  for (size_t i = 0; i < SCALE_LEN; ++i)
  {
    buzzer.tone(SCALE[i]);
    buzzer.print();
    delay(300);
  }

  buzzer.silence();
  DEBUG_PRINTLN("[Buzzer] silence");
  delay(800);
  
}
