#include <Arduino.h>

namespace config
{
  constexpr int kFlexPins[] = {A0, A1, A2, A3, A4};
  constexpr size_t kNumFlexSensors = sizeof(kFlexPins) / sizeof(kFlexPins[0]);
  constexpr int kFlexThreshold = 3600;
  constexpr int kBaudRate = 115200; 
}

void setupFlexPins();
void readFlexPins();

void setup()
{
  Serial.begin(config::kBaudRate);
  setupFlexPins();
}

void loop()
{
  readFlexPins();
  delay(1000); 
}

void setupFlexPins()
{
  for (size_t i = 0; i < config::kNumFlexSensors; ++i)
  {
    pinMode(config::kFlexPins[i], INPUT);
  }
}

void readFlexPins()
{
  printf("=========== FLEX SENSORS ===========");
  for (size_t i = 0; i < config::kNumFlexSensors; ++i)
  {
    int flex_value = analogRead(config::kFlexPins[i]);
    bool is_over_threshold = flex_value > config::kFlexThreshold;

    printf("Flex Pin: A%d | Raw Value: %d | Flexed: %s\n", i, flex_value, is_over_threshold ? "YES" : "NO");
  }
  printf("=========== FLEX SENSORS ===========");
}
