#include <Arduino.h>
#include "../config.h"
#include "../actuators/rgb_actuator.h"

RgbActuator rgb;

struct ColorEntry { const char* name; uint8_t r, g, b; };

static const ColorEntry COLORS[] = {
  { "Red",     255,   0,   0 },
  { "Green",     0, 255,   0 },
  { "Blue",      0,   0, 255 },
  { "White",   255, 255, 255 },
  { "Yellow",  255, 255,   0 },
  { "Cyan",      0, 255, 255 },
  { "Magenta", 255,   0, 255 },
  { "Off",       0,   0,   0 },
};
static const size_t COLORS_LEN = sizeof(COLORS) / sizeof(ColorEntry);

void setup()
{
  DEBUG_INIT();
  DEBUG_PRINTLN("=== RGB Actuator Test Script ===");
  rgb.setup();
}

void loop()
{
  for (size_t i = 0; i < COLORS_LEN; ++i)
  {
    const ColorEntry& c = COLORS[i];
    DEBUG_PRINTF("[RGB] -> %s\n", c.name);
    rgb.setColor(c.r, c.g, c.b);
    rgb.print();
    delay(800);
  }
}
