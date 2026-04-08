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
static constexpr int           BEEP_FREQ = 440;

static void setStateColor(HandState s)
{
  switch (s)
  {
    case HandState::RELEASE: rgb.setColor(  0,   0, 255); break; // Blue
    case HandState::GRAB:    rgb.setColor(255, 255,   0); break; // Yellow
    case HandState::POUR:    rgb.setColor(  0, 255, 255); break; // Cyan
    case HandState::SHAKE:   rgb.setColor(255,   0, 255); break; // Magenta
    case HandState::SERVE:   rgb.setColor(255, 255, 255); break; // White
  }
}

void setup()
{
  DEBUG_INIT();

  imu.setup();
  buzzer.setup();
  rgb.setup();
  rgb.setColor(255, 0, 0);  // Red — not yet connected

  DEBUG_PRINTLN("Calibrating flex sensors, hold hand flat...");
  flex.setup();
  flex.calibrate();
  DEBUG_PRINTLN("Calibration done. Ready.");

  // Rising arpeggio: C5 → E5 → G5 → C6
  const int notes[]         = { 523, 659, 784, 1047 };
  const int      durations[] = { 80, 80, 80, 160 };
  for (int i = 0; i < 4; ++i) {
    buzzer.tone(notes[i]);
    delay(durations[i]);
    buzzer.silence();
    delay(30);
  }

  ble.begin();
  // RGB stays red until a client connects
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
      const unsigned char id = static_cast<unsigned char>(state);

      const int* f = flex.getData();
      DEBUG_PRINTF("[Flex] %d %d %d %d\n", f[0], f[1], f[2], f[3]);
      DEBUG_PRINTF("[State] -> %s\n", handStateName(state));

      ble.sendState(id);

      buzzer.tone(BEEP_FREQ);
      buzzerOn    = true;
      buzzerOffAt = now + BEEP_MS;

      setStateColor(state);
    }
  }

  if (buzzerOn && now >= buzzerOffAt)
  {
    buzzer.silence();
    buzzerOn = false;
  }
}
