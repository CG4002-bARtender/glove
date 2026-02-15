#include "imu_sensor.h"
#include <Wire.h>

ImuSensor::ImuSensor()
    : Sensor(config::IMU_INTERVAL_MS),
      ax(0), ay(0), az(0),
      gx(0), gy(0), gz(0) {}

void ImuSensor::setup()
{
  Wire.begin();
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
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}

void ImuSensor::print()
{
  DEBUG_PRINTF("=========== IMU SENSOR ===========\n");
  DEBUG_PRINTF("[Accel] x:%+6d  y:%+6d  z:%+6d\n", ax, ay, az);
  DEBUG_PRINTF("[Gyro]  x:%+6d  y:%+6d  z:%+6d\n", gx, gy, gz);
  DEBUG_PRINTF("==================================\n");
}
