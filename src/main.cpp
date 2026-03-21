#include <Arduino.h>
#include "config.h"
#include "sensors/flex_sensor.h"
#include "sensors/imu_sensor.h"
#include "comms/ble_server.h"
#include "state/state_detector.h"
#include "state/hand_state.h"
#include "actuators/buzzer_actuator.h"
#include "actuators/rgb_actuator.h"

static FlexSensor     flex;
static ImuSensor      imu;
static BleServer      ble;
static StateDetector  detector;
static BuzzerActuator buzzer;
static RgbActuator    rgb;

static bool          buzzerOn    = false;
static unsigned long buzzerOffAt = 0;

static constexpr unsigned long BEEP_MS   = 80;
static constexpr uint16_t      BEEP_FREQ = 440;

void setup()
{
  DEBUG_INIT();

  ble.begin();
  imu.setup();
  buzzer.setup();
  rgb.setup();

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
      const uint8_t   id    = static_cast<uint8_t>(state);

      DEBUG_PRINTF("[State] -> %s\n", handStateName(state));

      if (ble.isConnected()) ble.sendState(id);

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

  ble.isConnected() ? rgb.off() : rgb.setColor(255, 0, 0);
}
