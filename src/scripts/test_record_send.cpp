#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include "../config.h"
#include "../sensors/mic_sensor.h"
#include "../comms/audio_recorder.h"

enum class State { IDLE, RECORDING, SENDING };

State g_state = State::IDLE;
MicSensor g_mic;
AudioRecorder g_recorder;

// Button debounce state
bool g_lastButtonState = HIGH;
unsigned long g_lastDebounceTime = 0;
bool g_buttonPressed = false;

void connectWifi();
void pollButton();
void handleIdle();
void handleRecording();
void handleSending();

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("\n=== Record & Send Test ===");

  pinMode(config::kRecordButtonPin, INPUT_PULLUP);

  // Allocate recording buffer BEFORE WiFi to avoid heap fragmentation
  Serial.printf("Max contiguous block: %d bytes\n", ESP.getMaxAllocHeap());
  if (!g_recorder.begin())
  {
    Serial.println("FATAL: Cannot allocate recording buffer. Halting.");
    while (true) { delay(1000); }
  }

  connectWifi();
  g_mic.setup();

  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("Press button to start recording...");
}

void loop()
{
  pollButton();

  switch (g_state)
  {
    case State::IDLE:      handleIdle();      break;
    case State::RECORDING: handleRecording(); break;
    case State::SENDING:   handleSending();   break;
  }
}

void connectWifi()
{
  Serial.printf("Connecting to WiFi: %s\n", config::kWifiSsid);
  WiFi.begin(config::kWifiSsid, config::kWifiPassword);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.printf("WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void pollButton()
{
  bool reading = digitalRead(config::kRecordButtonPin);
  unsigned long now = millis();

  if (reading != g_lastButtonState)
  {
    g_lastDebounceTime = now;
  }

  static bool stableState = HIGH;
  if ((now - g_lastDebounceTime) > config::kDebounceMs)
  {
    if (reading != stableState)
    {
      stableState = reading;
      if (stableState == LOW)
      {
        g_buttonPressed = true;
      }
    }
  }

  g_lastButtonState = reading;
}

void handleIdle()
{
  if (g_buttonPressed)
  {
    g_buttonPressed = false;
    g_recorder.reset();
    g_state = State::RECORDING;
    Serial.println(">> RECORDING STARTED (press again to stop)");
  }
}

void handleRecording()
{
  if (g_buttonPressed)
  {
    g_buttonPressed = false;
    Serial.printf(">> RECORDING STOPPED by button (%.2f sec)\n",
                  g_recorder.secondsRecorded());
    g_state = State::SENDING;
    return;
  }

  bool hasSpace = g_recorder.recordChunk();

  if (!hasSpace)
  {
    Serial.printf(">> RECORDING STOPPED: buffer full (%.2f sec)\n",
                  g_recorder.secondsRecorded());
    g_state = State::SENDING;
    return;
  }

  static unsigned long lastProgressMs = 0;
  unsigned long now = millis();
  if (now - lastProgressMs > 500)
  {
    lastProgressMs = now;
    Serial.printf("   Recording... %.1f sec (%d samples)\n",
                  g_recorder.secondsRecorded(), g_recorder.samplesRecorded());
  }
}

void handleSending()
{
  Serial.println(">> SENDING WAV...");

  bool ok = g_recorder.sendWav(config::kTcpServerHost, config::kTcpServerPort);

  if (ok)
    Serial.println(">> SEND COMPLETE. Returning to idle.");
  else
    Serial.println(">> SEND FAILED. Returning to idle.");

  g_recorder.reset();
  g_state = State::IDLE;
  Serial.println("Press button to start recording...");
}
