#include <Arduino.h>
#include <ArduinoJson.h>
#include "../config.h"
#include "../comms/mqtt_client.h"

MqttClient g_mqtt(config::kMqttBroker, config::kMqttPort, config::kMqttClientId);

unsigned long g_lastPublishMs = 0;
int g_messageCount = 0;

void createDummyData(JsonDocument& doc);

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== MQTT Test Script ===");

  if (!g_mqtt.connect(config::kWifiSsid, config::kWifiPassword))
  {
    Serial.println("Failed to connect. Restarting...");
    delay(5000);
    ESP.restart();
  }
}

void loop()
{
  g_mqtt.loop();

  unsigned long now = millis();
  if (now - g_lastPublishMs >= config::kMqttPublishIntervalMs)
  {
    g_lastPublishMs = now;

    JsonDocument doc;
    createDummyData(doc);

    g_mqtt.publish(config::kMqttTopic, doc);
  }
}

void createDummyData(JsonDocument& doc)
{
  doc["msg_id"] = g_messageCount++;
  doc["timestamp"] = millis();

  JsonArray flex = doc["flex"].to<JsonArray>();
  for (int i = 0; i < 5; i++)
    flex.add(random(2000, 4000));

  JsonObject imu = doc["imu"].to<JsonObject>();
  JsonArray accel = imu["accel"].to<JsonArray>();
  accel.add(random(-1000, 1000));
  accel.add(random(-1000, 1000));
  accel.add(16000 + random(-500, 500));

  JsonArray gyro = imu["gyro"].to<JsonArray>();
  gyro.add(random(-100, 100));
  gyro.add(random(-100, 100));
  gyro.add(random(-100, 100));
}
