#include "FreeRTOS.h"
#include "stepper.h"
#include <esp/gpio.h>
#include "task.h"

#define delay_ms(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)

void init_stepper(int _pin_1n1, int _pin_1n2, int _pin_1n3, int _pin_1n4) {
  pin_1n1 = _pin_1n1;
  pin_1n2 = _pin_1n2;
  pin_1n3 = _pin_1n3;
  pin_1n4 = _pin_1n4;

  gpio_enable(pin_1n1, GPIO_OUTPUT);
  gpio_enable(pin_1n2, GPIO_OUTPUT);
  gpio_enable(pin_1n3, GPIO_OUTPUT);
  gpio_enable(pin_1n4, GPIO_OUTPUT);
};

void step( int count) {
  while ( count > 0 ) {
    for (int i = 0; i < 8; i++)
    {
      setOutput(i);
      delay_ms(motorSpeed);
    }
    count--;
  }

  while ( count < 0 ) {
    for (int i = 7; i >= 0; i--)
    {
      setOutput(i);
      delay_ms(motorSpeed);
    }
    count++;
  }
};

void setOutput(int out) {
  // gpio_write(pin_1n1, lookup[out & 0x7]);
  // gpio_write(pin_1n2, lookup[out & 0x7]);
  // gpio_write(pin_1n3, lookup[out & 0x7]);
  // gpio_write(pin_1n4, lookup[out & 0x7]);
  gpio_write(pin_1n1, bitRead(lookup[out], 0));
  gpio_write(pin_1n2, bitRead(lookup[out], 1));
  gpio_write(pin_1n3, bitRead(lookup[out], 2));
  gpio_write(pin_1n4, bitRead(lookup[out], 3));
};
