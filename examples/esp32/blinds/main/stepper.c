#include <freertos/FreeRTOS.h>
#include "stepper.h"
#include <driver/gpio.h>
#include <freertos/task.h>

#define delay_ms(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)

int lookup[8] = { 0b0001, 0b011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001 };
int motorSpeed = 1200;  //variable to set stepper speed
int countsperrev = 512; // number of steps per full revolution

void init_stepper(int _pin_1n1, int _pin_1n2, int _pin_1n3, int _pin_1n4) {
  pin_1n1 = _pin_1n1;
  pin_1n2 = _pin_1n2;
  pin_1n3 = _pin_1n3;
  pin_1n4 = _pin_1n4;

  gpio_set_direction(pin_1n1, GPIO_MODE_OUTPUT);
  gpio_set_direction(pin_1n2, GPIO_MODE_OUTPUT);
  gpio_set_direction(pin_1n3, GPIO_MODE_OUTPUT);
  gpio_set_direction(pin_1n4, GPIO_MODE_OUTPUT);
};

void step(int count) {
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
  gpio_set_level(pin_1n1, bitRead(lookup[out], 0));
  gpio_set_level(pin_1n2, bitRead(lookup[out], 1));
  gpio_set_level(pin_1n3, bitRead(lookup[out], 2));
  gpio_set_level(pin_1n4, bitRead(lookup[out], 3));
};

int bitRead(unsigned char b, int bitPos)
{
  int x = b & (1 << bitPos);
  return x == 0 ? 0 : 1;
}
