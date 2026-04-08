#include <Arduino.h>
#include "../config.h"
#include "../sensors/flex_sensor.h"
#include "../sensors/imu_sensor.h"
#include "../state/state_detector.h"
#include "../state/hand_state.h"
#include "../actuators/buzzer_actuator.h"

static FlexSensor    flex;
static ImuSensor     imu;
static StateDetector detector;
static BuzzerActuator buzzer;

static bool          buzzerOn    = false;
static unsigned long buzzerOffAt = 0;

static constexpr unsigned long BEEP_MS   = 80;
static constexpr uint16_t      BEEP_FREQ = 440;

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== Gesture Test Script ===");

  imu.setup();
  buzzer.setup();

  DEBUG_PRINTLN("Calibrating flex sensors, hold hand flat...");
  flex.setup();
  flex.calibrate();
  DEBUG_PRINTLN("Calibration done. Ready.");
}

void loop()
{
  const unsigned long now = millis();

  if (flex.shouldRead(now)) flex.read();

  if (imu.shouldRead(now))
  {
    imu.read();

    if (detector.update(flex.getData(), imu.getData()))
    {
      const HandState state = detector.current();
      const int* f = flex.getData();
      DEBUG_PRINTF("[Flex] %d %d %d %d\n", f[0], f[1], f[2], f[3]);
      DEBUG_PRINTF("[Gesture] -> %s\n", handStateName(state));

      buzzer.tone(BEEP_FREQ);
      buzzerOn    = true;
      buzzerOffAt = now + BEEP_MS;
    }
  }

  if (buzzerOn && now >= buzzerOffAt)
  {
    buzzer.silence();
    buzzerOn = false;
  }
}
