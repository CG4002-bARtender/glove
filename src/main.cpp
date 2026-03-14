#include <Arduino.h>
#include "config.h"
#include "sensors/flex_sensor.h"
#include "sensors/imu_sensor.h"
#include "comms/ble_server.h"
#include "detection/activity_detector.h"
#include "actuators/buzzer_actuator.h"
#include "actuators/rgb_actuator.h"

static FlexSensor flex;
static ImuSensor imu;
static BleServer ble;
static ActivityDetector detector;
static BuzzerActuator buzzer;
static RgbActuator rgb;

struct Color
{
  uint8_t r, g, b;
};

static const Color GESTURE_COLORS[10] = {
    {255, 20, 147},  // 0 Hot Pink
    {0, 255, 0},     // 1 Green
    {0, 0, 255},     // 2 Blue
    {255, 255, 0},   // 3 Yellow
    {0, 255, 255},   // 4 Cyan
    {255, 0, 255},   // 5 Magenta
    {255, 128, 0},   // 6 Orange
    {128, 0, 255},   // 7 Purple
    {0, 255, 128},   // 8 Teal
    {255, 255, 255}, // 9 White
};

// ── Feedback state machine ────────────────────────────────────────────────────
enum class FeedbackState
{
  IDLE,
  SEND_BEEP,
  WAITING,
  ACK_BEEP_1,
  ACK_GAP,
  ACK_BEEP_2,
  COLOR_FLASH
};

static FeedbackState fbState = FeedbackState::IDLE;
static unsigned long fbAt = 0;
static uint8_t gestureId = 0; // cycles 0–9

static constexpr unsigned long SEND_BEEP_MS = 120;
static constexpr unsigned long ACK_WAIT_MS = 1000;
static constexpr unsigned long ACK_BEEP_1_MS = 80;
static constexpr unsigned long ACK_GAP_MS = 40;
static constexpr unsigned long ACK_BEEP_2_MS = 80;
static constexpr unsigned long COLOR_FLASH_MS = 600;

static constexpr uint16_t FREQ_SEND = 440;  // A4  — "sending"
static constexpr uint16_t FREQ_ACK_1 = 523; // C5  — ACK low note
static constexpr uint16_t FREQ_ACK_2 = 784; // G5  — ACK high note

void updateFeedback(unsigned long now);

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
  unsigned long now = millis();

  if (flex.shouldRead(now)) flex.read();

  if (imu.shouldRead(now))
  {
    imu.read();

    GloveSample s;
    const int *f = flex.getData();
    const int16_t *d = imu.getData();

    uint8_t bits = 0;
    for (int i = 0; i < 5; ++i)
      if (f[i] >= config::flex::THRESHOLDS[i])
        bits |= (1u << i);
    s.flex_bits = bits;

    DEBUG_PRINTF("[Flex] P:%d R:%d M:%d I:%d T:%d  (raw: %d %d %d %d %d)\n",
                 (bits >> 0) & 1, (bits >> 1) & 1, (bits >> 2) & 1, (bits >> 3) & 1, (bits >> 4) & 1,
                 f[0], f[1], f[2], f[3], f[4]);

    for (int i = 0; i < 3; ++i)
      s.accel[i] = d[i];
    for (int i = 0; i < 3; ++i)
      s.gyro[i] = d[3 + i];

    if (detector.update(s) && ble.isConnected())
    {
      const size_t len = detector.gestureLength();
      const GloveSample *data = detector.gestureData();

      ble.sendGestureWindow(reinterpret_cast<const uint8_t *>(data), len);

      if (fbState == FeedbackState::IDLE)
      {
        buzzer.tone(FREQ_SEND);
        fbState = FeedbackState::SEND_BEEP;
        fbAt = now;
        DEBUG_PRINTLN("[Feedback] gesture sent");
      }
    }
  }

  updateFeedback(now);

  if (fbState == FeedbackState::IDLE)
  {
    ble.isConnected() ? rgb.off() : rgb.setColor(255, 0, 0);
  }
}

void updateFeedback(unsigned long now)
{

  switch (fbState)
  {
  case FeedbackState::IDLE:
    break;

  case FeedbackState::SEND_BEEP:
    if (now - fbAt >= SEND_BEEP_MS)
    {
      buzzer.silence();
      fbState = FeedbackState::WAITING;
      fbAt = now;
    }
    break;

  case FeedbackState::WAITING:
    if (now - fbAt >= ACK_WAIT_MS)
    {
      // Mocked ACK — in production replace with real server response
      buzzer.tone(FREQ_ACK_1);
      fbState = FeedbackState::ACK_BEEP_1;
      fbAt = now;
    }
    break;

  case FeedbackState::ACK_BEEP_1:
    if (now - fbAt >= ACK_BEEP_1_MS)
    {
      buzzer.silence();
      fbState = FeedbackState::ACK_GAP;
      fbAt = now;
    }
    break;

  case FeedbackState::ACK_GAP:
    if (now - fbAt >= ACK_GAP_MS)
    {
      buzzer.tone(FREQ_ACK_2);
      fbState = FeedbackState::ACK_BEEP_2;
      fbAt = now;
    }
    break;

  case FeedbackState::ACK_BEEP_2:
    if (now - fbAt >= ACK_BEEP_2_MS)
    {
      buzzer.silence();
      const Color &c = GESTURE_COLORS[gestureId];
      rgb.setColor(c.r, c.g, c.b);
      DEBUG_PRINTF("[Feedback] gesture %u -> color flash\n", gestureId);
      gestureId = (gestureId + 1) % 10;
      fbState = FeedbackState::COLOR_FLASH;
      fbAt = now;
    }
    break;

  case FeedbackState::COLOR_FLASH:
    if (now - fbAt >= COLOR_FLASH_MS)
    {
      rgb.off();
      fbState = FeedbackState::IDLE;
    }
    break;
  }
}