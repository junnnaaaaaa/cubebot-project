#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cmath>
#include <iostream>
#include <stdio.h>
using namespace std;

class stepper {
private:
  const uint8_t coils[4];
  const uint8_t full_step[4] = {0b1100, 0b0110, 0b0011, 0b1001};
  const uint32_t steps_per_rotation;

public:
  stepper(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint32_t spr)
      : coils{pin1, pin2, pin3, pin4},
        steps_per_rotation(spr) { // assigns and pins
    for (int i = 0; i < 4; i++) {
      // assigns gpio pins and puts them to nothing
      gpio_init(coils[i]);
      gpio_set_dir(coils[i], GPIO_OUT);
      gpio_put(coils[i], 1);
      sleep_ms(300);
      gpio_put(coils[i], 0);
    }
  };
  void move(float turns, float speed, bool dir = false) { // temp manual delay
    uint32_t steps = steps_per_rotation * turns;
    uint64_t delay_us = ((60 / speed) / steps_per_rotation) * 1000000;
    for (int i = 0; i < steps; i++) {
      int current_step = i % size(full_step);
      for (int j = 0; j < 4; j++) {
        int x = dir ? j : 3 - j;
        gpio_put(coils[j], (full_step[current_step] >> x) & 1);
        // gpio_put(coils[1], 1);
      }
      sleep_us(delay_us);
      for (int i = 0; i < 4; i++) {
        gpio_put(coils[i], 0);
      }
    }
  }
  void test_coils() {
    for (int i = 0; i < 4; i++) {
      gpio_put(coils[i], 1);
    }
    sleep_ms(1000);
    for (int i = 0; i < 4; i++) {
      gpio_put(coils[i], 0);
    }
  }
};
int main() {
  stdio_init_all();
  stepper test1(0, 1, 2, 3, 2048);
  stepper drive_test(4, 5, 6, 7, 2048);
  while (true) {
    test1.move(1, 22, true);
    drive_test.move(1, 15);
    sleep_ms(500);
    // gpio_put(0, 1);
  }
  return 0;
}
