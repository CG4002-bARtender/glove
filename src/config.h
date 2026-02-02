#pragma once

#include <Arduino.h>

namespace config
{
  // Serial
  constexpr int kBaudRate = 9600;

  // Blink
  constexpr int kLedPin = 2;
  constexpr int kBlinkDelayMs = 500;  

  // Flex sensor config
  constexpr int kFlexPins[] = {A0, A1, A2, A3, A4};
  constexpr size_t kNumFlexSensors = sizeof(kFlexPins) / sizeof(kFlexPins[0]);
  constexpr unsigned long kFlexIntervalMs = 1000;
  constexpr int kFlexThreshold = 3600;

  // IMU config
  constexpr unsigned long kImuIntervalMs = 20;  // 50Hz

  // WiFi config
  constexpr const char* kWifiSsid = "YOUR_WIFI_SSID";
  constexpr const char* kWifiPassword = "YOUR_WIFI_PASSWORD";

  // MQTT config
  constexpr const char* kMqttBroker = "broker.hivemq.com";
  constexpr int kMqttPort = 1883;
  constexpr const char* kMqttClientId = "esp32_glove";
  constexpr const char* kMqttTopic = "sensors";
  constexpr unsigned long kMqttPublishIntervalMs = 2000;
  constexpr int kMqttYieldTimeoutMs = 10;
  constexpr int kMqttKeepAliveSec = 60;
}
