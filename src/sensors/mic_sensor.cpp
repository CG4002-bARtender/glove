#include "mic_sensor.h"

MicSensor::MicSensor() : Sensor(config::kMicIntervalMs), samples(), bytes_read(0) {}

MicSensor::~MicSensor()
{
    i2s_driver_uninstall(I2S_NUM_0);
}

void MicSensor::setup()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = config::kI2S_SCK,
        .ws_io_num = config::kI2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = config::kI2S_SD
    };

    esp_err_t err;
    
    err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    Serial.printf("I2S driver install: %s\n", err == ESP_OK ? "OK" : "FAILED");

    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    Serial.printf("I2S pin config: %s\n", err == ESP_OK ? "OK" : "FAILED");
}

void MicSensor::read()
{
  esp_err_t err = i2s_read(I2S_NUM_0, &samples, sizeof(samples), &bytes_read, 1000);
  if (err != ESP_OK) {
    bytes_read = 0;
    Serial.printf("Failed to read!");
  }
}

void MicSensor::print()
{    
  if (bytes_read > 0) {
    // Print first sample and last sample
    int samples_read = bytes_read / 4;
    Serial.printf("Bytes: %d | First: %d | Last: %d | ", 
                  bytes_read, samples[0], samples[samples_read-1]);
    
    // Check if ANY non-zero values
    bool has_data = false;
    for (int i = 0; i < samples_read; i++) {
      if (samples[i] != 0) {
        has_data = true;
        break;
      }
    }
    Serial.println(has_data ? "HAS DATA ✓" : "ALL ZEROS ✗");
  }
}
