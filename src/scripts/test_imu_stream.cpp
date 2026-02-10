#include <Arduino.h>
#include <ArduinoJson.h>
#include "../config.h"
#include "../sensors/imu_sensor.h"
#include "../comms/mqtt_client.h"

ImuSensor g_imu;
MqttClient g_mqtt(
    config::kMqttBroker,
    config::kMqttPort,
    config::kMqttClientId,
    config::kImuStreamPublishIntervalMs,
    config::kMqttUsername,
    config::kMqttPassword);

void setup()
{
  Serial.begin(config::kBaudRate);
  delay(1000);
  Serial.println("=== IMU Stream Test ===");

  g_imu.setup();

  if (!g_mqtt.connect(config::kWifiSsid, config::kWifiPassword))
  {
    Serial.println("MQTT connect failed. Restarting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Streaming IMU data...");
}

void loop()
{
  g_mqtt.loop();

  unsigned long now = millis();

  if (g_imu.shouldRead(now))
  {
    g_imu.read();
  }

  if (g_mqtt.shouldPublish(now))
  {
    JsonDocument doc;
    doc["t"] = millis();

    JsonArray a = doc["a"].to<JsonArray>();
    a.add(g_imu.getAx());
    a.add(g_imu.getAy());
    a.add(g_imu.getAz());

    JsonArray g = doc["g"].to<JsonArray>();
    g.add(g_imu.getGx());
    g.add(g_imu.getGy());
    g.add(g_imu.getGz());

    g_mqtt.publish(config::kImuStreamTopic, doc);
  }
}
