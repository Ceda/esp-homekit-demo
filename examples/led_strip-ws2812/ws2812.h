#ifndef ws2812_h
#define ws2812_h

#include "ws2812_i2s/ws2812_i2s.h"

#define MD_STATIC           0 // solid color
#define MD_STRIPES          1 // rotation of color with white
#define MD_COMPLIMENT       2 // rotation of color with complementary
#define MD_RAINBOW			3 // rotation of W,B,G,R,Y,M,C
#define MD_COMET            4 // comets of color
#define MD_COMET_STRIPES    5 // comets of color with white
#define MD_COMET_COMPLIMENT 6 // comets of color with complementary
#define MD_COMET_RAINBOW    7 // comets of W,B,G,R,Y,M,C
//#define MD_FIREWORKS		8 // fireworks of W,B,G,R,Y,M,C

void ws2812_init(int pixel_number);

void ws2812_on(bool on);

void ws2812_setColor(ws2812_pixel_t color);

void ws2812_setBrightness(int brightness);

void ws2812_setMode(int mode_index);

void ws2812_setSpeed(int speed);

void ws2812_setInverted(bool inverted);

#endif
