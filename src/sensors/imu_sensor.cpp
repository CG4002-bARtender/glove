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

  if (m_connected)
  {
    Serial.println("MPU6050 connected successfully");
  }
  else
  {
    Serial.println("MPU6050 connection failed!");
  }
}

void ImuSensor::read()
{
  if (!m_connected || !m_mpu.testConnection())
  {
    Serial.println("Failed to read IMU data: MPU is disconnected.");
    return;
  }

  m_mpu.getMotion6(
      &m_data.accel[0], &m_data.accel[1], &m_data.accel[2],
      &m_data.gyro[0], &m_data.gyro[1], &m_data.gyro[2]);
}
