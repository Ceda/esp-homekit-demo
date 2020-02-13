#include <stdio.h>
#include <string.h>
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
#include "multipwm.h"

#include "wifi.h"

#define MAX_SERVICES 20
#define LPF_SHIFT 4  // divide by 16
#define LPF_INTERVAL 10  // in milliseconds

const uint8_t led_gpios[] = {
        5, 12, 15, 13
};

const uint8_t led_indexes[] = {
        0, 1, 2, 3
};

uint16_t led_brightness[] = {
        100, 100, 100, 100
};

uint8_t led_states[] = {
        true, true, true, true
};

pwm_info_t pwm_info;

const size_t led_count = sizeof(led_gpios) / sizeof(*led_gpios);

static void wifi_init() {
        struct sdk_station_config wifi_config = {
                .ssid = WIFI_SSID,
                .password = WIFI_PASSWORD,
        };

        sdk_wifi_set_opmode(STATION_MODE);
        sdk_wifi_station_set_config(&wifi_config);
        sdk_wifi_station_connect();
}

void led_strip_send() {
        multipwm_stop(&pwm_info);

        uint8_t led_0_brightness = led_states[0] ? (led_brightness[0] > 0 ? led_brightness[0] : 100) : 0;
        uint8_t led_1_brightness = led_states[1] ? (led_brightness[1] > 0 ? led_brightness[1] : 100) : 0;
        uint8_t led_2_brightness = led_states[2] ? (led_brightness[2] > 0 ? led_brightness[2] : 100) : 0;
        uint8_t led_3_brightness = led_states[3] ? (led_brightness[3] > 0 ? led_brightness[3] : 100) : 0;

        multipwm_set_duty(&pwm_info, 0, UINT16_MAX - UINT16_MAX * led_0_brightness / 100);
        multipwm_set_duty(&pwm_info, 1, UINT16_MAX - UINT16_MAX * led_1_brightness / 100);
        multipwm_set_duty(&pwm_info, 2, UINT16_MAX - UINT16_MAX * led_2_brightness / 100);
        multipwm_set_duty(&pwm_info, 3, UINT16_MAX - UINT16_MAX * led_3_brightness / 100);
        multipwm_start(&pwm_info);
        printf ("sending r=%d, g=%d, b=%d, w=%d,\n", led_brightness[0], led_brightness[1], led_brightness[2], led_brightness[3]);
}

void lightSET_task(void *pvParameters) {
        led_strip_send();
        vTaskDelete(NULL);
}

void lightSET() {
        xTaskCreate(lightSET_task, "Light Set", 256, NULL, 2, NULL);
}

void led_strip_init() {
        pwm_info.channels = led_count;
        pwm_info.freq = 1000;
        pwm_info.reverse = true;

        multipwm_init(&pwm_info);

        for (uint8_t i=0; i<pwm_info.channels; i++) {
                multipwm_set_pin(&pwm_info, i, led_gpios[i]);
        }

        lightSET();
}

void dimmer_write(int index, int brightness) {
        if (led_states[index]) {
                led_brightness[index] = brightness;

                printf("Setting brightness on relay %d to %d\n", index, brightness);
        } else {
                led_brightness[index] = 0;

                printf("Setting brightness on relay %d to %d\n", index, 0);
        }

        lightSET();
}

void relay_write(int index, bool on) {
        led_states[index] = on;

        lightSET();

        printf("Setting state on relay %d %s\n", index, on ? "true" : "false");
}



void led_identify_task(void *_args) {
        printf("LED identify\n");

        // led_brightness[0] = 0;
        // led_brightness[1] = 0;
        // led_brightness[2] = 0;
        // led_brightness[3] = 0;
        //
        // led_strip_send();
        //
        // vTaskDelay(100 / portTICK_PERIOD_MS);
        //
        // led_brightness[0] = 100;
        // led_brightness[1] = 100;
        // led_brightness[2] = 100;
        // led_brightness[3] = 100;
        //
        // led_strip_send();


        // uint8_t stored_state = led_brightness;
        // // uint8_t off_state[] = { 0, 0, 0, 0 };
        // // uint8_t on_state[] = { 100, 100, 100, 100};
        //
        // for (int i=0; i<3; i++) {
        //     for (int j=0; j<2; j++) {
        //         // led_brightness = { 0, 0, 0, 0 };
        //         vTaskDelay(100 / portTICK_PERIOD_MS);
        //
        //         // led_brightness = { 100, 100, 100, 100};
        //         vTaskDelay(100 / portTICK_PERIOD_MS);
        //     }
        //
        //     vTaskDelay(250 / portTICK_PERIOD_MS);
        // }
        //
        // led_brightness = stored_state;

        vTaskDelete(NULL);
}

void led_identify(homekit_value_t _value) {
        xTaskCreate(led_identify_task, "LED identify", 128, NULL, 2, NULL);
}

void dimmer_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
        uint8_t *index = context;
        dimmer_write(*index, value.int_value);
}

void relay_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
        uint8_t *index = context;
        relay_write(*index, value.bool_value);
}

homekit_accessory_t *accessories[2];

homekit_server_config_t config = {
        .accessories = accessories,
        .password = "111-11-111"
};

void init_accessory() {
        uint8_t macaddr[6];
        sdk_wifi_get_macaddr(STATION_IF, macaddr);

        int name_len = snprintf(NULL, 0, "Relays-%02X%02X%02X",
                                macaddr[3], macaddr[4], macaddr[5]);
        char *name_value = malloc(name_len+1);
        snprintf(name_value, name_len+1, "Relays-%02X%02X%02X",
                 macaddr[3], macaddr[4], macaddr[5]);

        homekit_service_t* services[MAX_SERVICES + 1];
        homekit_service_t** s = services;

        *(s++) = NEW_HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
                NEW_HOMEKIT_CHARACTERISTIC(NAME, name_value),
                NEW_HOMEKIT_CHARACTERISTIC(MANUFACTURER, "HaPK"),
                NEW_HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0"),
                NEW_HOMEKIT_CHARACTERISTIC(MODEL, "Relays"),
                NEW_HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
                NEW_HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
                NULL
        });

        for (int i=0; i < led_count; i++) {
                int relay_name_len = snprintf(NULL, 0, "Relay %d", i + 1);
                char *relay_name_value = malloc(relay_name_len+1);
                snprintf(relay_name_value, relay_name_len+1, "Relay %d", i + 1);

                *(s++) = NEW_HOMEKIT_SERVICE(LIGHTBULB, .characteristics=(homekit_characteristic_t*[]) {
                        NEW_HOMEKIT_CHARACTERISTIC(NAME, relay_name_value),
                        NEW_HOMEKIT_CHARACTERISTIC(
                                ON,
                                true,
                                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(
                                        relay_callback, .context=(void*)&led_indexes[i]
                                        ),
                                ),
                        NEW_HOMEKIT_CHARACTERISTIC(
                                BRIGHTNESS, 100,
                                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(
                                        dimmer_callback, .context=(void*)&led_indexes[i]
                                        ),
                                ),
                        NULL
                });
        }

        *(s++) = NULL;

        accessories[0] = NEW_HOMEKIT_ACCESSORY(.category=homekit_accessory_category_other, .services=services);
        accessories[1] = NULL;
}

void user_init(void) {
        uart_set_baud(0, 115200);

        init_accessory();
        wifi_init();
        led_strip_init();

        homekit_server_init(&config);
}
