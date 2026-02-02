#include "flex_sensor.h"

FlexSensor::FlexSensor() : Sensor(config::kFlexIntervalMs), m_data{} {}

void FlexSensor::setup()
{
    for (size_t i = 0; i < config::kNumFlexSensors; ++i)
    {
        pinMode(config::kFlexPins[i], INPUT);   
    }
}

void FlexSensor::read()
{
    Serial.printf("=========== FLEX SENSORS ===========\n");
    for (size_t i = 0; i < config::kNumFlexSensors; ++i)
    {
        int flex_value = analogRead(config::kFlexPins[i]);

        Serial.printf("[Flex Pin: A%d]: %d", i, flex_value);

        m_data.flex_values[i] = flex_value;
    }
    Serial.printf("=====================================\n");
}
