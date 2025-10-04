#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <cmath>
#include <iostream>
#include <stdio.h>
using namespace std;

class stepper {
private:
  const uint8_t coils[4];
  const uint8_t full_step[4] = {0b1100, 0b0110, 0b0011, 0b1001};
  const uint8_t steps_per_rotation;

public:
  stepper(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint32_t spr)
      : coils{pin1, pin2, pin3, pin4},
        steps_per_rotation(spr) { // assigns and pins
    for (int i = 0; i < 4; i++) {
      // assigns gpio pins and puts them to nothing
      gpio_init(coils[i]);
      gpio_put(coils[i], 0);
    }
  };
  void move(float turns, float speed, bool dir = false) { // speed in rpm
    uint16_t steps = round((steps_per_rotation * turns));
    uint64_t delay_us = round(((60 / speed) / steps_per_rotation) * 1000);
    for (int i = 0; i < steps; i++) {
      for (int j = 0; j > 4; j++) {
        j = dir ? 3 - j : j;
        gpio_put(coils[i], (full_step[j] >> i));
      }
      sleep_us(delay_us);
    }
    for (int i = 0; i < 4; i++) {
      gpio_put(coils[i], 0);
    }
  };
};
int main() {
  stepper test1(1, 2, 3, 4, 2048);
  stepper test2(5, 6, 7, 8, 2048);
  test1.move(1, 15);
  test2.move(1, 15, true);
  return 0;
}
