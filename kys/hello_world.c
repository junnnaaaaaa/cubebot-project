#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/uart.h"
#include <stdio.h>
#include <stdlib.h>

#define IN1 0
#define IN2 1
#define IN3 6
#define IN4 7

const uint8_t STEPS[4] = {
  0b0001,
  0b0010,
  0b0100,
  0b1000,
};

void move_stepper(int steps, int step_delay) {
  static uint phase = 0;
  int dir = (steps > 0) ? 1 : -1 ;
  steps = abs(steps);

  for(int i = 0; i<steps; i++) {
    phase = (phase + dir + 4) % 4;

    gpio_put(IN1, STEPS[phase] & 1);  
    gpio_put(IN2, STEPS[phase] & 2);  
    gpio_put(IN3, STEPS[phase] & 4);  
    gpio_put(IN4, STEPS[phase] & 8);  

    busy_wait_us(step_delay);
    
  }
  gpio_put(IN1, 0); gpio_put(IN2, 0);  
  gpio_put(IN3, 0); gpio_put(IN4, 0);  
}
int main() {
  gpio_init(IN1); gpio_set_dir(IN1, GPIO_OUT);  
  gpio_init(IN2); gpio_set_dir(IN2, GPIO_OUT);  
  gpio_init(IN3); gpio_set_dir(IN3, GPIO_OUT);  
  gpio_init(IN4); gpio_set_dir(IN4, GPIO_OUT);  

  //start with coils off
  gpio_put_masked(0b1111 << 2, 0);
  // Add this before main loop:  
  gpio_put(IN1, 1); sleep_ms(300); gpio_put(IN1, 0);  
  gpio_put(IN2, 1); sleep_ms(300); gpio_put(IN2, 0);  
  gpio_put(IN3, 1); sleep_ms(300); gpio_put(IN3, 0);  
  gpio_put(IN4, 1); sleep_ms(300); gpio_put(IN4, 0);  
  while(1) {  
        move_stepper(2048, 2000);  // 1 full rotation forward  
        sleep_ms(1000);         // rest 1 sec  
        move_stepper(-2048, 2000); // 1 rotation backward  
        sleep_ms(1000);  
    }  
}
