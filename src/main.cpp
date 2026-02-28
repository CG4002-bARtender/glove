#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors/imu_sensor.h"
#include "sensors/flex_sensor.h"
#include "comms/mqtt_client.h"

ImuSensor  g_imu;
FlexSensor g_flex;
MqttClient g_mqtt(
  config::MQTT_BROKER, config::MQTT_PORT, config::MQTT_CLIENT_ID,
  config::MQTT_PUBLISH_INTERVAL_MS, config::MQTT_USERNAME, config::MQTT_PASSWORD
);

void calibrateFlex()
{
  DEBUG_PRINTLN("=== Flex Calibration ===");
  DEBUG_PRINTLN("Keep glove FLAT and still...");

  for (int i = 3; i >= 1; --i)
  {
    DEBUG_PRINTF("  Starting in %d...\n", i);
    delay(1000);
  }

  DEBUG_PRINTLN("Sampling for 5 seconds, hold still!");
  g_flex.calibrate(5000);

  DEBUG_PRINTLN("Calibration done. Baselines:");
  const float* baseline = g_flex.getBaseline();
  for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
    DEBUG_PRINTF("  Finger %d: %.1f\n", i, baseline[i]);
  DEBUG_PRINTLN("========================");
}

void setup()
{
  DEBUG_INIT();
  delay(1000);
  DEBUG_PRINTLN("=== Glove Starting ===");

  g_imu.setup();
  g_flex.setup();
  calibrateFlex();

  if (!g_mqtt.connect(config::WIFI_SSID, config::WIFI_PASSWORD))
  {
    DEBUG_PRINTLN("MQTT connect failed. Restarting...");
    delay(5000);
    ESP.restart();
  }
}

void loop()
{
  g_mqtt.loop();

  unsigned long now = millis();

  if (g_imu.shouldRead(now))  g_imu.read();
  if (g_flex.shouldRead(now)) g_flex.read();

  if (g_mqtt.shouldPublish(now))
  {
    bool isFlexed[config::FLEX_SENSOR_PINS_LEN];
    g_flex.getIsFlexed(isFlexed);
    const int16_t* imuRaw = g_imu.getRaw(); // ax,ay,az,gx,gy,gz

    JsonDocument doc;
    doc["msg_id"]    = g_mqtt.getMessageCount();
    doc["timestamp"] = now;

    JsonArray flexArr = doc["flex"].to<JsonArray>();
    for (size_t i = 0; i < config::FLEX_SENSOR_PINS_LEN; ++i)
      flexArr.add(isFlexed[i]);

    JsonObject imu  = doc["imu"].to<JsonObject>();
    JsonArray accel = imu["accel"].to<JsonArray>();
    accel.add(imuRaw[0]); accel.add(imuRaw[1]); accel.add(imuRaw[2]);
    JsonArray gyro  = imu["gyro"].to<JsonArray>();
    gyro.add(imuRaw[3]);  gyro.add(imuRaw[4]);  gyro.add(imuRaw[5]);

    g_mqtt.publish(config::MQTT_TOPIC, doc);
  }
}
