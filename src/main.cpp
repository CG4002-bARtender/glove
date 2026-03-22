#include <Arduino.h>
#include "config.h"
#include "sensors/flex_sensor.h"
#include "sensors/imu_sensor.h"
#include "comms/mqtt_client.h"
#include "state/state_detector.h"
#include "state/hand_state.h"
#include "actuators/buzzer_actuator.h"
#include "actuators/rgb_actuator.h"

static FlexSensor     flex;
static ImuSensor      imu;
static MqttClient     mqtt(
  config::mqtt::BROKER,
  config::mqtt::PORT,
  config::mqtt::CLIENT_ID,
  0,  // event-driven, no interval
  config::mqtt::USERNAME,
  config::mqtt::PASSWORD
);
static StateDetector  detector;
static BuzzerActuator buzzer;
static RgbActuator    rgb;

static bool          buzzerOn    = false;
static unsigned long buzzerOffAt = 0;

static constexpr unsigned long BEEP_MS   = 80;
static constexpr uint16_t      BEEP_FREQ = 440;

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
  const uint16_t notes[]    = { 523, 659, 784, 1047 };
  const int      durations[] = { 80, 80, 80, 160 };
  for (int i = 0; i < 4; ++i) {
    buzzer.tone(notes[i]);
    delay(durations[i]);
    buzzer.silence();
    delay(30);
  }

  mqtt.connect(config::wifi::SSID, config::wifi::PASSWORD);
  rgb.setColor(0, 255, 0);  // Green — MQTT connected, idle
}

void loop()
{
  const unsigned long now = millis();

  mqtt.loop();

  if (flex.shouldRead(now)) flex.read();

  if (imu.shouldRead(now))
  {
    imu.read();

    if (detector.update(flex.getData(), imu.getData()))
    {
      const HandState state = detector.current();
      const uint8_t   id    = static_cast<uint8_t>(state);

      DEBUG_PRINTF("[State] -> %s\n", handStateName(state));

      mqtt.publish(config::mqtt::TOPIC_STATE, &id, 1);

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
