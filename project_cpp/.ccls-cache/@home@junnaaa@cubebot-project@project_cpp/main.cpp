#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cmath>
#include <iostream>
#include <stdio.h>
using namespace std;
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

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
void hc05init() {
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  uart_init(UART_ID, BAUD_RATE);
  uart_puts(UART_ID, "\nbluetooth initialised\n");
  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
}
int main() {
  bool blue_init = false;
  stdio_init_all();
  stepper driver(2, 3, 4, 5, 2048);
  gpio_set_input_enabled(28, true);
  while (!blue_init) {
    if (gpio_get(28)) {
      hc05init();
      blue_init = true;
    }
  }
  uart_puts(UART_ID, "\nfirst loop left\n");
  while (1) {
    char ch;
    if (uart_is_readable(UART_ID)) {
      ch = uart_getc(UART_ID);
      uart_puts(UART_ID, "\nchar is read\n");
    }
    if (ch == 'l') {
      driver.move(1, 15);
      ch = 'x';
    }
    if (ch == 'r') {
      driver.move(1, 15, true);
      ch = 'x';
    }
  }
  uart_puts(UART_ID, "\nsecond loop left\n");
  return 0;
}
