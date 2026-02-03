#include "mqtt_client.h"
#include "../config.h"

MqttClient::MqttClient(const char* broker, int port, const char* clientId, unsigned long publishIntervalMs,
                       const char* username, const char* password)
    : m_wifiClient(),
      m_client(m_wifiClient),
      m_broker(broker),
      m_port(port),
      m_clientId(clientId),
      m_username(username),
      m_password(password),
      m_publishIntervalMs(publishIntervalMs),
      m_lastPublishMs(0),
      m_messageCount(0)
{
  m_wifiClient.setInsecure();
  m_client.setServer(m_broker, m_port);
}

bool MqttClient::connect(const char* ssid, const char* password)
{
  connectWifi(ssid, password);
  return connectMqtt();
}

bool MqttClient::isConnected()
{
  return m_client.connected();
}

void MqttClient::loop()
{
  if (!isConnected())
  {
    Serial.println("MQTT disconnected. Reconnecting...");
    connectMqtt();
  }
  m_client.loop();
}

bool MqttClient::shouldPublish(unsigned long now)
{
  if (now - m_lastPublishMs >= m_publishIntervalMs)
  {
    m_lastPublishMs = now;
    return true;
  }
  return false;
}

bool MqttClient::publish(const char* topic, const JsonDocument& doc)
{
  char buffer[config::kMqttJsonBufferSize];
  serializeJson(doc, buffer);
  return publish(topic, buffer);
}

bool MqttClient::publish(const char* topic, const char* payload)
{
  if (!isConnected())
  {
    Serial.println("Cannot publish: not connected");
    return false;
  }

  bool success = m_client.publish(topic, payload);
  if (!success)
  {
    Serial.println("Publish failed");
    return false;
  }

  Serial.printf("Published to %s: %s\n", topic, payload);
  m_messageCount++;
  return true;
}

void MqttClient::connectWifi(const char* ssid, const char* password)
{
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.printf("WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
}

bool MqttClient::connectMqtt()
{
  Serial.printf("Connecting to MQTT: %s:%d\n", m_broker, m_port);

  bool success;
  if (m_username != nullptr && m_password != nullptr)
  {
    success = m_client.connect(m_clientId, m_username, m_password);
  }
  else
  {
    success = m_client.connect(m_clientId);
  }

  if (!success)
  {
    Serial.printf("MQTT connect failed, state=%d\n", m_client.state());
    return false;
  }

  Serial.println("MQTT connected!");
  return true;
}
