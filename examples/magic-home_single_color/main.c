/*
 * Example of using esp-homekit library to control
 * a simple $5 Sonoff Basic using HomeKit.
 * The esp-wifi-config library is also used in this
 * example. This means you don't have to specify
 * your network's SSID and password before building.
 *
 * In order to flash the sonoff basic you will have to
 * have a 3,3v (logic level) FTDI adapter.
 *
 * To flash this example connect 3,3v, TX, RX, GND
 * in this order, beginning in the (square) pin header
 * next to the button.
 * Next hold down the button and connect the FTDI adapter
 * to your computer. The sonoff is now in flash mode and
 * you can flash the custom firmware.
 *
 * WARNING: Do not connect the sonoff to AC while it's
 * connected to the FTDI adapter! This may fry your
 * computer and sonoff.
 *
   modified to do pulse-width-modulation (PWM) of LED
 */

#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include <pwm.h>

const int led_gpio = 12;
const int button_gpio = 0;

float brightness;
bool light_on = false;
uint8_t pins[1];

void gpio_init() {
        pins[0] = led_gpio;
        pwm_init(1, pins, false);
}

void lightSET_task(void *pvParameters) {
        int value;
        if (light_on) {
                value = (UINT16_MAX*brightness/100);
                pwm_set_duty(value);
                printf("ON  %3d [%5d]\n", (int)brightness, value);
        } else {
                printf("OFF\n");
                pwm_set_duty(0);
        }
        vTaskDelete(NULL);
}

void lightSET() {
        xTaskCreate(lightSET_task, "Light Set", 256, NULL, 2, NULL);
}

void light_init() {
        printf("light_init:\n");
        light_on = false;
        brightness = 0;
        printf("light_on = false  brightness = 100 %%\n");
        pwm_set_freq(1000);
        printf("PWMpwm_set_freq = 1000 Hz  pwm_set_duty = 0 = 0%%\n");
        pwm_set_duty(0);
        pwm_start();
        lightSET();
}


homekit_value_t light_on_get() {
        return HOMEKIT_BOOL(light_on);
}

void light_on_set(homekit_value_t value) {
        if (value.format != homekit_format_bool) {
                printf("Invalid light_on-value format: %d\n", value.format);
                return;
        }
        light_on = value.bool_value;
        lightSET();
}

homekit_value_t brightness_get() {
        return HOMEKIT_INT(brightness);
}

void brightness_set(homekit_value_t value) {
        if (value.format != homekit_format_int) {
                printf("Invalid brightness-value format: %d\n", value.format);
                return;
        }
        brightness = value.int_value;
        lightSET();
}


void light_identify(homekit_value_t _value) {
        printf("Light Identify\n");
}


homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Sonoff Dimmer");

homekit_characteristic_t lightbulb_on = HOMEKIT_CHARACTERISTIC_(ON, false, .getter=light_on_get, .setter=light_on_set);

homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(
                .id=1,
                .category=homekit_accessory_category_switch,
                .services=(homekit_service_t*[]){
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION,
                                .characteristics=(homekit_characteristic_t*[]){
                        &name,
                        HOMEKIT_CHARACTERISTIC(MANUFACTURER, "iTEAD"),
                        HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "PWM Dimmer"),
                        HOMEKIT_CHARACTERISTIC(MODEL, "Sonoff Basic"),
                        HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1.6"),
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(LIGHTBULB, .primary=true,
                                .characteristics=(homekit_characteristic_t*[]){
                        HOMEKIT_CHARACTERISTIC(NAME, "Sonoff Dimmer"),
                        &lightbulb_on,
                        HOMEKIT_CHARACTERISTIC(BRIGHTNESS, 100, .getter=brightness_get, .setter=brightness_set),
                        NULL
                }),
                NULL
        }),
        NULL
};


homekit_server_config_t config = {
        .accessories = accessories,
        .password = "111-11-111"
};

void on_wifi_ready() {
        homekit_server_init(&config);
}

void create_accessory_name() {
        uint8_t macaddr[6];
        sdk_wifi_get_macaddr(STATION_IF, macaddr);

        int name_len = snprintf(NULL, 0, "Sonoff Dimmer %02X:%02X:%02X",
                                macaddr[3], macaddr[4], macaddr[5]);
        char *name_value = malloc(name_len+1);
        snprintf(name_value, name_len+1, "Sonoff Dimmer %02X:%02X:%02X",
                 macaddr[3], macaddr[4], macaddr[5]);

        name.value = HOMEKIT_STRING(name_value);
}

void user_init(void) {
        uart_set_baud(0, 115200);
        create_accessory_name();

        wifi_config_init("Sonoff Dimmer", NULL, on_wifi_ready);

        gpio_init();
        light_init();
}
