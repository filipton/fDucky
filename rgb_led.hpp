#ifndef RGB_LED_HPP
#define RGB_LED_HPP

#define PIXEL_PIN 12
#define PIXEL_POWER_PIN 11
static Adafruit_NeoPixel rgb_led(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setPixelColor(uint32_t color)
{
  rgb_led.clear();
  rgb_led.setPixelColor(0, color);
  rgb_led.show();
}

void flashError(bool *code)
{
  for (int i = 0; i < 3; i++)
  {
    setPixelColor(rgb_led.Color(code[i] ? 255 : 0, 0, code[i] ? 0 : 255));
    delay(250);
    setPixelColor(rgb_led.Color(0, 0, 0));
    delay(250);
  }
  delay(500);
}

#endif