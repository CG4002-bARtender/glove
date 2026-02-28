#pragma once

#include <Arduino.h>

// Set to 0 to disable all debug serial prints for production
#define DEBUG_MODE 1

#if DEBUG_MODE
  #define DEBUG_INIT()        Serial.begin(config::BAUD_RATE)
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_INIT()        ((void)0)
  #define DEBUG_PRINT(...)    ((void)0)
  #define DEBUG_PRINTLN(...)  ((void)0)
  #define DEBUG_PRINTF(...)   ((void)0)
#endif

namespace config
{
  // Serial
  constexpr int         BAUD_RATE = 115200;

  // Blink config
  constexpr int         INBUILT_LED_PIN = 2;
  constexpr int         BLINK_DELAY_MS = 500;

  // IMU config
  constexpr int         I2C_SDA_PIN = 22;     
  constexpr int         I2C_SCL_PIN = 21;     
  constexpr int         I2C_CLOCK_SPEED = 100000;
  constexpr int         IMU_INTERVAL_MS = 33;

  // Flex config
  constexpr int         FLEX_SENSOR_PINS[] = {A0,A1,A2,A3,A4};
  constexpr size_t      FLEX_SENSOR_PINS_LEN = sizeof(FLEX_SENSOR_PINS) / sizeof(int);
  constexpr int         FLEX_SENSOR_CALIBRATION_ROUNDS = 30;
  constexpr int         FLEX_SENSOR_CALIBRATION_DELAY = 100;
  constexpr int         FLEX_INTERVAL_MS = 330;
  constexpr float       FLEX_FLEXED_THRESHOLD = 100.0f; // ADC counts above baseline = flexed

  // WiFi config
  constexpr const char* WIFI_SSID = "Medea";
  constexpr const char* WIFI_PASSWORD = "12345678";

  // MQTT config
  constexpr const char* MQTT_BROKER = "k12141b9.ala.eu-central-1.emqxsl.com";
  constexpr int         MQTT_PORT = 8883;
  constexpr const char* MQTT_USERNAME = "test";  
  constexpr const char* MQTT_PASSWORD = "test";  
  constexpr const char* MQTT_CLIENT_ID = "esp32_glove";
  constexpr const char* MQTT_TOPIC = "glove";
  constexpr int         MQTT_PUBLISH_INTERVAL_MS = 2000;
  constexpr size_t      MQTT_CHUNK_SIZE = 1100;
  constexpr size_t      MQTT_JSON_BUFFER_SIZE = 512;
  constexpr size_t      MQTT_QUEUE_SIZE = 80; 
}
