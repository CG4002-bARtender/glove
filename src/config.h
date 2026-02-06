#pragma once

#include <Arduino.h>

namespace config
{
  // Serial
  constexpr int kBaudRate = 115200;

  // Blink
  constexpr int kLedPin = 2;
  constexpr int kBlinkDelayMs = 500;  

  // Flex sensor config
  constexpr int kFlexPins[] = {A0, A1, A2, A3, A4};
  constexpr size_t kNumFlexSensors = sizeof(kFlexPins) / sizeof(kFlexPins[0]);
  constexpr unsigned long kFlexIntervalMs = 1000;

  // IMU config
  constexpr unsigned long kImuIntervalMs = 20;  // 50Hz

  // Mic config
  constexpr unsigned long kMicIntervalMs = 20;
  constexpr int kI2S_WS = 17;
  constexpr int kI2S_SCK = 14;
  constexpr int kI2S_SD = 27;

  // WiFi config
  constexpr const char* kWifiSsid = "IphoneAlam";
  constexpr const char* kWifiPassword = "pasuhagu";

  // MQTT config
  constexpr const char* kMqttBroker = "k12141b9.ala.eu-central-1.emqxsl.com";
  constexpr int kMqttPort = 8883;
  constexpr const char* kMqttUsername = "test";  
  constexpr const char* kMqttPassword = "test";  
  constexpr const char* kMqttClientId = "esp32_glove";
  constexpr const char* kMqttTopic = "glove";
  constexpr unsigned long kMqttPublishIntervalMs = 2000;
  constexpr int kMqttYieldTimeoutMs = 10;
  constexpr int kMqttKeepAliveSec = 60;
  constexpr size_t kMqttJsonBufferSize = 512;
}
