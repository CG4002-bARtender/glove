#include "imu_sensor.h"
#include <Wire.h>

ImuSensor::ImuSensor()
    : Sensor(config::kImuIntervalMs), m_mpu(), m_data{}, m_connected(false)
{
}

void ImuSensor::setup()
{
  Wire.begin();
  m_mpu.initialize();
  m_connected = m_mpu.testConnection();
}

void ImuSensor::read()
{
  while (!m_connected)
  {
    Serial.println("Lost connection with IMU. Attempting to reconnect...");
    m_connected = m_mpu.testConnection();
  }

  m_mpu.getMotion6(
      &m_data.accel[0], &m_data.accel[1], &m_data.accel[2],
      &m_data.gyro[0], &m_data.gyro[1], &m_data.gyro[2]);

  Serial.printf("=============== IMU ================\n");
  Serial.printf("[Accel] X: %6d  Y: %6d  Z: %6d\n", m_data.accel[0], m_data.accel[1], m_data.accel[2]);
  Serial.printf("[Gyro]  X: %6d  Y: %6d  Z: %6d\n", m_data.gyro[0], m_data.gyro[1], m_data.gyro[2]);
  Serial.printf("====================================\n");
}
