#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>

class MqttClient
{
public:
  MqttClient(const char* broker, int port, const char* clientId);

  bool connect(const char* ssid, const char* password);
  bool isConnected();
  void loop();

  bool publish(const char* topic, const JsonDocument& doc);
  bool publish(const char* topic, const char* payload);

private:
  void connectWifi(const char* ssid, const char* password);
  bool connectMqtt();

  WiFiClient m_wifiClient;
  IPStack m_ipstack;
  MQTT::Client<IPStack, Countdown> m_client;

  const char* m_broker;
  int m_port;
  const char* m_clientId;
  bool m_connected;
};
