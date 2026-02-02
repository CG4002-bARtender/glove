#include "mqtt_client.h"

MqttClient::MqttClient(const char* broker, int port, const char* clientId)
    : m_wifiClient(),
      m_ipstack(m_wifiClient),
      m_client(m_ipstack),
      m_broker(broker),
      m_port(port),
      m_clientId(clientId),
      m_connected(false)
{
}

bool MqttClient::connect(const char* ssid, const char* password)
{
  connectWifi(ssid, password);
  return connectMqtt();
}

bool MqttClient::isConnected()
{
  return m_connected && m_client.isConnected();
}

void MqttClient::loop()
{
  if (!isConnected())
  {
    Serial.println("MQTT disconnected. Reconnecting...");
    connectMqtt();
  }
  m_client.yield(10);
}

bool MqttClient::publish(const char* topic, const JsonDocument& doc)
{
  char buffer[512];
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

  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.payload = (void*)payload;
  message.payloadlen = strlen(payload);

  int rc = m_client.publish(topic, message);
  if (rc != 0)
  {
    Serial.printf("Publish failed, rc=%d\n", rc);
    return false;
  }

  Serial.printf("Published to %s: %s\n", topic, payload);
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

  int rc = m_ipstack.connect((char*) m_broker, m_port);
  if (rc != 0)
  {
    Serial.printf("TCP connect failed, rc=%d\n", rc);
    m_connected = false;
    return false;
  }

  MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
  options.clientID.cstring = (char*)m_clientId;
  options.keepAliveInterval = 60;

  rc = m_client.connect(options);
  if (rc != 0)
  {
    Serial.printf("MQTT connect failed, rc=%d\n", rc);
    m_connected = false;
    return false;
  }

  Serial.println("MQTT connected!");
  m_connected = true;
  return true;
}
