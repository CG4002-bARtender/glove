#include "buzzer_actuator.h"

BuzzerActuator::BuzzerActuator() : _freq(0) {}

void BuzzerActuator::setup()
{
  ledcSetup(config::buzzer::LEDC_CH, 1000, config::buzzer::LEDC_RES);
  ledcAttachPin(config::buzzer::PIN, config::buzzer::LEDC_CH);
  silence();
}

void BuzzerActuator::tone(uint16_t freq)
{
  _freq = freq;
  ledcWriteTone(config::buzzer::LEDC_CH, freq);
}

void BuzzerActuator::silence()
{
  _freq = 0;
  ledcWriteTone(config::buzzer::LEDC_CH, 0);
}

void BuzzerActuator::print()
{
  DEBUG_PRINTF("[Buzzer] freq: %u Hz\n", _freq);
}
