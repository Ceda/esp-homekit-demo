#include "ws2812.h"

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>

// B G R W order on struct (ignoring white value)
#define RAINBOW_N 7
ws2812_pixel_t RAINBOW[RAINBOW_N] = {
	{{255,255,255,  0}}, // white
	{{255,  0,  0,  0}}, // blue
	{{  0,255,  0,  0}}, // green
	{{  0,  0,255,  0}}, // red
	{{  0,255,255,  0}}, // yellow
	{{255,  0,255,  0}}, // magenta
	{{255,255,  0,  0}}  // cyan
};

const int COMET = 15;

int _led_count;

ws2812_pixel_t *pixels;
ws2812_pixel_t *black;

bool _running = true;
int _position = -1;
float _brightness = 1.0f;
ws2812_pixel_t _solid_color 		= {{255,255,255,  0}}; // white
ws2812_pixel_t _complement_color 	= {{  0,  0,  0,  0}}; // black
ws2812_pixel_t _colors[2] = {{{255,255,255,  0}}, {{  0,  0,  0,  0}}};
ws2812_pixel_t _stripes[2] = {{{255,255,255,  0}}, {{255,255,255,  0}}};
int _mode_index = MD_STATIC;
bool _inverted = false;
int _mode_delay = 0;

int constrain(int input, int min, int max) {
	if (input < min) return min;
	if (input > max) return max;
	return input;
}

void setPixel(int index, ws2812_pixel_t color, float brightnessMod) {
	if (_inverted) index = (_led_count - 1) - index;
	pixels[index].red = color.red * _brightness * brightnessMod;
	pixels[index].green = color.green * _brightness * brightnessMod;
	pixels[index].blue = color.blue * _brightness * brightnessMod;
}

void rotation(int color_count, ws2812_pixel_t colors[]) {
	_position = (_position + 1) % color_count % _led_count;

	for (int i = 0; i < _led_count; i++) {
		int colorIndex = (i + _position) % color_count;

		setPixel(i, colors[colorIndex], 1.0f);
	}

	ws2812_i2s_update(pixels, PIXEL_RGB);
}

void comets(int color_count, ws2812_pixel_t colors[]) {
	_position = (_position + 1) % ((color_count * COMET) % _led_count);
	int colorIndex = (_position / COMET) % color_count;

	for (int i = 0; i < _led_count; i++) {
		int mod = (i + _position) % COMET;
		if (mod == 0 && i > 0) colorIndex = (colorIndex + 1) % color_count;
		
		// W 2 2 4 4 8 8 16 16 32 32 64 64 128 128
		if (mod == 0) {
			setPixel(i, RAINBOW[0], 1.0f);
		}
		else {
			float fade = 1 / (float) pow(2, floor(mod / 2));
			setPixel(i, colors[colorIndex], fade);
		}
	}
	ws2812_i2s_update(pixels, PIXEL_RGB);
}

void ws2812_service(void *_args) {
	uint32_t now = 0;
	uint32_t last_call_time = 0;

	while (true) {
		if (_running) {
			now = xTaskGetTickCount() * portTICK_PERIOD_MS;
			
			if (now - last_call_time > _mode_delay) {
				last_call_time = now;

				switch (_mode_index) {
					case MD_STATIC:
						rotation(1, _colors);
						break;
					case MD_STRIPES:
						rotation(2, _stripes);
						break;
					case MD_COMPLIMENT:
						rotation(2, _colors);
						break;
					case MD_RAINBOW:
						rotation(RAINBOW_N, RAINBOW);
						break;
					case MD_COMET:
						comets(1, _colors);
						break;
					case MD_COMET_STRIPES:
						comets(2, _stripes);
						break;
					case MD_COMET_COMPLIMENT:
						comets(2, _colors);
						break;
					case MD_COMET_RAINBOW:
						comets(RAINBOW_N, RAINBOW);
						break;
					default:
						rotation(1, _colors);
				}
			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void ws2812_init(int pixel_number) {
	_led_count = pixel_number;

	pixels = (ws2812_pixel_t*) malloc(_led_count * sizeof(ws2812_pixel_t));
	black = (ws2812_pixel_t*) malloc(_led_count * sizeof(ws2812_pixel_t));

	for (int i = 0; i < _led_count; i++) {
		black[i].red = 0;
		black[i].green = 0;
		black[i].blue = 0;
	}
	
	ws2812_i2s_init(_led_count, PIXEL_RGB);

	xTaskCreate(ws2812_service, "ws2812Service", 255, NULL, 2, NULL);
}

void ws2812_on(bool on) {
	_running = on;
	if (!_running) {
		ws2812_i2s_update(black, PIXEL_RGB);
	}
}

void ws2812_setColor(ws2812_pixel_t color) {
	_solid_color.red = color.red;
	_solid_color.green = color.green;
	_solid_color.blue = color.blue;
	
	_complement_color.red = 255 - _solid_color.red;
	_complement_color.green = 255 - _solid_color.green;
	_complement_color.blue = 255 - _solid_color.blue;
	
	_colors[0] = _solid_color;
	_colors[1] = _complement_color;
	_stripes[0] = _solid_color;
}

void ws2812_setBrightness(int brightness) {
	_brightness = brightness / 100.0f;
}

void ws2812_setMode(int mode_index) {
	_mode_index = mode_index;
}

void ws2812_setSpeed(int speed) {
	// max delay 250ms, min delay 0ms
	_mode_delay = speed * -2.5f + 250;
}

void ws2812_setInverted(bool inverted) {
	_inverted = inverted;
}
