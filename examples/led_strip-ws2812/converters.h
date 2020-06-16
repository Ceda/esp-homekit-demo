#ifndef converters_h
#define converters_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ws2812_i2s/ws2812_i2s.h"

//int min(int a, int b) {
//  return a < b ? a : b;
//}

//int max(int a, int b) {
//  return a > b ? a : b;
//}

//int limit(int value, int lower, int upper) {
//  return min(max(value, lower), upper);
//}

int map(int value, int in_lower, int in_upper, int out_lower, int out_upper) {
  return out_lower + ((out_upper - out_lower) / (in_upper - in_lower)) * (value - in_lower);
}

//https://github.com/espressif/esp-idf/issues/83
double __ieee754_remainder(double x, double y) {
	return x - y * floor(x/y);
}

// https://en.wikipedia.org/wiki/HSL_and_HSV
float hsv_f(float n, int h, float s, float v) {
  float k = fmod(n + h/60.0f, 6);
  return v - v * s * fmax(fmin(fmin(k, 4-k), 1), 0);
}
void hs2rgb(int h, float s, ws2812_pixel_t *rgb) {
  float v = 1;
  rgb->red = 255 * hsv_f(5, h, s, v);
  rgb->green = 255 * hsv_f(3, h, s, v);
  rgb->blue = 255 * hsv_f(1, h, s, v);
  rgb->white = 0;
//  printf("h:%03d s:%.2f rgb:#%02x%02x%02x\n", h, s, rgb->red, rgb->green, rgb->blue);
}

#endif
