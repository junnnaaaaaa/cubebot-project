#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iostream>

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

class stepper {
  // creates stepper object
private:
  // gpio port for each coil
  const uint8_t coils[4];
  // stepper phases
  const uint8_t full_step[4] = {0b1100, 0b0110, 0b0011, 0b1001};
  const uint32_t steps_per_rotation;

public:
  stepper(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint32_t spr)
      : coils{pin1, pin2, pin3, pin4},
        steps_per_rotation(spr) { // assigns constants including pins
    for (int i = 0; i < 4; i++) {
      // initialises pins associated with each coil, test each coil
      gpio_init(coils[i]);
      gpio_set_dir(coils[i], GPIO_OUT);
      gpio_put(coils[i], 1);
      sleep_ms(300);
      gpio_put(coils[i], 0);
    }
  };
  void move(float turns, float speed,
            bool reverse = false) { // temp manual delay
    uint32_t steps = steps_per_rotation * turns;
    uint64_t delay_us = ((60 / speed) / steps_per_rotation) * 1000000;
    for (uint i = 0; i < steps; i++) {
      // finds which phase based on the steps
      int current_step = i % std::size(full_step);
      for (int j = 0; j < 4; j++) {
        // adjusts phase if the direction is reversed
        int x = reverse ? j : 3 - j;
        // assigns gpio port to respective bit from step constant
        gpio_put(coils[j], (full_step[current_step] >> x) & 1);
      }
      sleep_us(delay_us); // pause between each step
    }
    for (int i = 0; i < 4; i++) { // turns off coils
      gpio_put(coils[i], 0);
    }
  }
  void test_coils() { // test function, ignore
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
  // sets gpio ports to uart and initiliases and sets up uart connection
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  uart_init(UART_ID, BAUD_RATE);
  uart_puts(UART_ID, "\nuart connection initialised\n");
  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
}
class robot {
  // initialises class to manage steppers
private:
  stepper steppers[5];
  // letters for each move
  const char letter_order[5] = {'r', 'l', 'f', 'b', 'd'};

public:
  robot(stepper stepr, stepper stepl, stepper stepf, stepper stepb,
        stepper stepd)
      : steppers{stepr, stepl, stepf, stepb, stepd} {};
  void single_char() { // function to deal with one character
    char move;
    bool reverse;
    bool is_turned = false;
    if (uart_is_readable(UART_ID)) {
      move = uart_getc(UART_ID);
      reverse = isupper(move); // checks case of character
      move =
          reverse ? tolower(move) : move; // ensures the character is lowercase
      for (uint i = 0; i < sizeof(letter_order); i++) {
        // goes through each letter of letter order array and checks matching
        if (move == letter_order[i]) {
          // goes a bit over for better alingment, to be tuned
          steppers[i].move(0.28, 15, reverse);
          sleep_ms(20);
          // goes back a bit less to compensate for missing steps. to be tuned
          steppers[i].move(0.2, 15, !reverse);
          is_turned = true;
          break;
        }
      }
      if (!is_turned) { // simple feedback to user if input is good
        uart_puts(UART_ID, "\nbad letter\n");
      } else {
        uart_puts(UART_ID, "\ngood letter\n");
      }
    }
  }
};
int main() {
  stdio_init_all();
  bool blue_init = false;
  // enables uart detection pin
  gpio_set_input_enabled(28, true);
  // sets up each stepper
  stepper r_face(2, 3, 4, 5, 2048);
  stepper f_face(6, 7, 8, 9, 2048);
  stepper l_face(10, 11, 12, 13, 2048);
  stepper b_face(14, 15, 16, 17, 2048);
  stepper d_face(18, 19, 20, 21, 2048);
  // sets up robot class
  robot cube(r_face, l_face, f_face, b_face, d_face);
  while (!blue_init) { // loops until uart connection between bluetooth and pico
                       // is initialised
    if (gpio_get(28)) {
      hc05init();
      blue_init = true;
    }
  }
  uart_puts(UART_ID, "\nfirst loop left\n"); // trouble shooting
  while (true) {
    // constantly checks for next move
    cube.single_char();
    if (uart_is_readable(UART_ID) &&
        uart_getc(UART_ID) == 'x') { // exit condition
      uart_puts(UART_ID, "\nexiting program\n");
      break;
    }
    sleep_ms(50);
  }
  uart_puts(UART_ID, "\nexit done, thanks for using\n");
  return 0;
}
