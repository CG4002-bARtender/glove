#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class MqttClient
{
public:
  MqttClient(const char* broker, int port, const char* clientId, unsigned long publishIntervalMs,
             const char* username = nullptr, const char* password = nullptr);

  bool connect(const char* ssid, const char* wifiPassword);
  bool isConnected();
  void loop();

  bool shouldPublish(unsigned long now);
  bool publish(const char* topic, const JsonDocument& doc);
  bool publish(const char* topic, const char* payload);

  int getMessageCount() const { return m_messageCount; }

private:
  void connectWifi(const char* ssid, const char* password);
  bool connectMqtt();

  WiFiClientSecure m_wifiClient;
  PubSubClient m_client;

  const char* m_broker;
  int m_port;
  const char* m_clientId;
  const char* m_username;
  const char* m_password;

  unsigned long m_publishIntervalMs;
  unsigned long m_lastPublishMs;
  int m_messageCount;
};
