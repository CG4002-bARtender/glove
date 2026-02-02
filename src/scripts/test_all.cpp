#include <Arduino.h>
#include <ArduinoJson.h>
#include "../config.h"
#include "../sensors/imu_sensor.h"
#include "../sensors/flex_sensor.h"
#include "../comms/mqtt_client.h"

ImuSensor g_imuSensor;
FlexSensor g_flexSensor;
MqttClient g_mqtt(config::kMqttBroker, config::kMqttPort, config::kMqttClientId);

unsigned long g_lastPublishMs = 0;
int g_messageCount = 0;

void createSensorPayload(JsonDocument& doc);

void setup()
{
  Serial.begin(config::kBaudRate);
  Serial.println("=== Full Integration Test ===");
  Serial.println("Initializing sensors...");

  g_imuSensor.setup();
  g_flexSensor.setup();

  Serial.println("Connecting to WiFi and MQTT...");
  if (!g_mqtt.connect(config::kWifiSsid, config::kWifiPassword))
  {
    Serial.println("Failed to connect. Restarting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Setup complete. Starting data collection...");
}

void loop()
{
  g_mqtt.loop();

  unsigned long now = millis();

  if (g_imuSensor.shouldRead(now))
  {
    g_imuSensor.read();
  }

  if (g_flexSensor.shouldRead(now))
  {
    g_flexSensor.read();
  }

  if (now - g_lastPublishMs >= config::kMqttPublishIntervalMs)
  {
    g_lastPublishMs = now;

    JsonDocument doc;
    createSensorPayload(doc);

    g_mqtt.publish(config::kMqttTopic, doc);
  }
}

void createSensorPayload(JsonDocument& doc)
{
  doc["msg_id"] = g_messageCount++;
  doc["timestamp"] = millis();

  const FlexData& flexData = g_flexSensor.getData();
  JsonArray flex = doc["flex"].to<JsonArray>();
  for (size_t i = 0; i < config::kNumFlexSensors; i++)
  {
    flex.add(flexData.flex_values[i]);
  }

  const ImuData& imuData = g_imuSensor.getData();
  JsonObject imu = doc["imu"].to<JsonObject>();

  JsonArray accel = imu["accel"].to<JsonArray>();
  accel.add(imuData.accel[0]);
  accel.add(imuData.accel[1]);
  accel.add(imuData.accel[2]);

  JsonArray gyro = imu["gyro"].to<JsonArray>();
  gyro.add(imuData.gyro[0]);
  gyro.add(imuData.gyro[1]);
  gyro.add(imuData.gyro[2]);
}
