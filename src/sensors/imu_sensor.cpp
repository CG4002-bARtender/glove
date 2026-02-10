#include "imu_sensor.h"
#include <Wire.h>

ImuSensor::ImuSensor()
    : Sensor(config::kImuIntervalMs),
      ax(0), ay(0), az(0),
      gx(0), gy(0), gz(0) {}

void ImuSensor::setup()
{
  Wire.begin();
  Wire.setClock(100000);
  mpu.initialize();

  if (!mpu.testConnection())
  {
    Serial.println("MPU6050 connection failed!");
    while (1)
      ;
  }
  Serial.println("MPU6050 connected.");
}

void ImuSensor::read()
{
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}

void ImuSensor::print()
{
  Serial.printf("=========== IMU SENSOR ===========\n");
  Serial.printf("[Accel] x:%+6d  y:%+6d  z:%+6d\n", ax, ay, az);
  Serial.printf("[Gyro]  x:%+6d  y:%+6d  z:%+6d\n", gx, gy, gz);
  Serial.printf("==================================\n");
}
