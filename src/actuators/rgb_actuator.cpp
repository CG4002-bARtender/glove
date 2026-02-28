#include "rgb_actuator.h"

RgbActuator::RgbActuator() : _r(0), _g(0), _b(0) {}

void RgbActuator::setup()
{
  ledcSetup(config::rgb::LEDC_CH_R, config::rgb::LEDC_FREQ, config::rgb::LEDC_RES);
  ledcSetup(config::rgb::LEDC_CH_G, config::rgb::LEDC_FREQ, config::rgb::LEDC_RES);
  ledcSetup(config::rgb::LEDC_CH_B, config::rgb::LEDC_FREQ, config::rgb::LEDC_RES);

  ledcAttachPin(config::rgb::PIN_R, config::rgb::LEDC_CH_R);
  ledcAttachPin(config::rgb::PIN_G, config::rgb::LEDC_CH_G);
  ledcAttachPin(config::rgb::PIN_B, config::rgb::LEDC_CH_B);

  off();
}

void RgbActuator::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  _r = r; _g = g; _b = b;
  ledcWrite(config::rgb::LEDC_CH_R, r);
  ledcWrite(config::rgb::LEDC_CH_G, g);
  ledcWrite(config::rgb::LEDC_CH_B, b);
}

void RgbActuator::off()
{
  setColor(0, 0, 0);
}

void RgbActuator::print()
{
  DEBUG_PRINTF("[RGB] R:%u G:%u B:%u\n", _r, _g, _b);
}
