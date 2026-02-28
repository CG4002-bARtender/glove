#include "imu_sensor.h"
#include <Wire.h>

ImuSensor::ImuSensor()
    : Sensor(config::IMU_INTERVAL_MS),
      values{} {}

void ImuSensor::setup()
{
  Wire.begin(config::I2C_SDA_PIN, config::I2C_SCL_PIN);
  Wire.setClock(config::I2C_CLOCK_SPEED);
  mpu.initialize();

  while (!mpu.testConnection())
  {
    DEBUG_PRINTLN("MPU6050 connection failed!");
  }
  DEBUG_PRINTLN("MPU6050 connected.");
}

void ImuSensor::read()
{
  mpu.getMotion6(&values[0], &values[1], &values[2], &values[3], &values[4], &values[5]);
}

void ImuSensor::print()
{
  DEBUG_PRINTF("=========== IMU SENSOR ===========\n");
  DEBUG_PRINTF("[Accel] x:%+6d  y:%+6d  z:%+6d\n", values[0], values[1], values[2]);
  DEBUG_PRINTF("[Gyro]  x:%+6d  y:%+6d  z:%+6d\n", values[3], values[4], values[5]);
  DEBUG_PRINTF("==================================\n");
}
