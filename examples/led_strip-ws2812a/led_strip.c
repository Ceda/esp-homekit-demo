/*
 * HomeKit interface for ws2812 led strip.
 *
 * Derived from the led_strip_animation example
 * in https://github.com/maximkulkin/esp-homekit-demo
 */
#include <stdio.h>
#include <stdlib.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <math.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "libs/FastLED/FastLED.h"
#include "wifi.h"

#define LED_COUNT 16      // this is the number of WS2812B leds on the strip
#define LED_INBUILT_GPIO 2 // this is the onboard LED used to show on/off only

#define UUID_MODE "1C52000A-457C-4D3C-AABA-E6F207422A10"
#define UUID_SPEED "4CB2534E-9C95-4C4F-A746-36238BAACE3A"

// Home Kit variables
bool hk_on = true;
float hk_hue = 0;
float hk_saturation = 0;
int hk_brightness = 100;
int hk_mode = 0;
int hk_speed = 100;

// static void wifi_init() {
//         struct sdk_station_config wifi_config = {
//                 .ssid = WIFI_SSID,
//                 .password = WIFI_PASSWORD,
//         };
//
//         sdk_wifi_set_opmode(STATION_MODE);
//         sdk_wifi_station_set_config(&wifi_config);
//         sdk_wifi_station_connect();
// }
//
// void led_identify_task(void *_args) {
//         // initialise the onboard led as a secondary indicator (handy for testing)
//         gpio_enable(LED_INBUILT_GPIO, GPIO_OUTPUT);
//
//         for (int i = 0; i < 3; i++) {
//                 for (int j = 0; j < 3; j++) {
//                         gpio_write(LED_INBUILT_GPIO, 0);
//                         vTaskDelay(100 / portTICK_PERIOD_MS);
//                         gpio_write(LED_INBUILT_GPIO, 1);
//                         vTaskDelay(100 / portTICK_PERIOD_MS);
//                 }
//                 vTaskDelay(250 / portTICK_PERIOD_MS);
//         }
//
//         gpio_write(LED_INBUILT_GPIO, 1);
//
//         vTaskDelete(NULL);
// }
//
// void led_identify(homekit_value_t _value) {
//         xTaskCreate(led_identify_task, "LED identify", 128, NULL, 2, NULL);
// }
//
// homekit_value_t led_on_get() {
//         return HOMEKIT_BOOL(hk_on);
// }
//
// void led_on_set(homekit_value_t value) {
//         hk_on = value.bool_value;
//
//         ws2812_on(hk_on);
// }
//
// homekit_value_t led_brightness_get() {
//         return HOMEKIT_INT(hk_brightness);
// }
//
// void led_brightness_set(homekit_value_t value) {
//         hk_brightness = value.int_value;
//
//         ws2812_setBrightness(hk_brightness);
// }
//
// homekit_value_t led_hue_get() {
//         return HOMEKIT_FLOAT(hk_hue);
// }
//
// void led_hue_set(homekit_value_t value) {
//         hk_hue = value.float_value;
//
//         ws2812_pixel_t color = { { 0, 0, 0, 0 } };
//         hs2rgb(hk_hue, hk_saturation / 100.0f, &color);
//
//         //printf("hue:%f sat:%f rgb:%02x%02x%02x\n", hk_hue, hk_saturation, color.red, color.green, color.blue);
//
//         ws2812_setColor(color);
// }
//
// homekit_value_t led_saturation_get() {
//         return HOMEKIT_FLOAT(hk_saturation);
// }
//
// void led_saturation_set(homekit_value_t value) {
//         hk_saturation = value.float_value;
//
//         ws2812_pixel_t color = { { 0, 0, 0, 0 } };
//         hs2rgb(hk_hue, hk_saturation / 100.0f, &color);
//
//         //printf("hue:%f sat:%f rgb:%02x%02x%02x\n", hk_hue, hk_saturation, color.red, color.green, color.blue);
//
//         ws2812_setColor(color);
// }
//
// // homekit_value_t led_mode_get() {
// //         return HOMEKIT_INT(hk_mode);
// // }
// //
// // void led_mode_set(homekit_value_t value) {
// //         hk_mode = value.int_value;
// //
// //         ws2812_setMode(hk_mode);
// // }
// //
// // homekit_value_t led_speed_get() {
// //         return HOMEKIT_INT(hk_speed);
// // }
// //
// // void led_speed_set(homekit_value_t value) {
// //         hk_speed = value.int_value;
// //
// //         ws2812_setInverted(hk_speed < 0);
// //         ws2812_setSpeed(abs(hk_speed));
// // }
//
// homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Chihiro");
//
// homekit_accessory_t *accessories[] = {
//         HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lightbulb, .services = (homekit_service_t*[]) {
//                 HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
//                         &name,
//                         HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Generic"),
//                         HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "137A2BABF19D"),
//                         HOMEKIT_CHARACTERISTIC(MODEL, "LEDStripFX"),
//                         HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
//                         HOMEKIT_CHARACTERISTIC(IDENTIFY, led_identify),
//                         NULL
//                 }),
//                 HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
//                         HOMEKIT_CHARACTERISTIC(NAME, "Chihiro"),
//                         HOMEKIT_CHARACTERISTIC(
//                                 ON, true,
//                                 .getter = led_on_get,
//                                 .setter = led_on_set
//                                 ),
//                         HOMEKIT_CHARACTERISTIC(
//                                 BRIGHTNESS, 100,
//                                 .getter = led_brightness_get,
//                                 .setter = led_brightness_set
//                                 ),
//                         HOMEKIT_CHARACTERISTIC(
//                                 HUE, 0,
//                                 .getter = led_hue_get,
//                                 .setter = led_hue_set
//                                 ),
//                         HOMEKIT_CHARACTERISTIC(
//                                 SATURATION, 0,
//                                 .getter = led_saturation_get,
//                                 .setter = led_saturation_set
//                                 ),
//                         // HOMEKIT_CHARACTERISTIC(
//                         //         CUSTOM,
//                         //         .type = UUID_MODE,
//                         //         .description = "fxMode",
//                         //         .format = homekit_format_int,
//                         //         .permissions = homekit_permissions_paired_read
//                         //                        | homekit_permissions_paired_write,
//                         //         .min_value = (float[]) {0},
//                         //         .max_value = (float[]) {7},
//                         //         .min_step = (float[]) {1},
//                         //         .value = HOMEKIT_INT_(0),
//                         //         .getter = led_mode_get,
//                         //         .setter = led_mode_set
//                         //         ),
//                         // HOMEKIT_CHARACTERISTIC(
//                         //         CUSTOM,
//                         //         .type = UUID_SPEED,
//                         //         .description = "Speed",
//                         //         .format = homekit_format_int,
//                         //         .permissions = homekit_permissions_paired_read
//                         //                        | homekit_permissions_paired_write,
//                         //         .min_value = (float[]) {-100},
//                         //         .max_value = (float[]) {100},
//                         //         .min_step = (float[]) {1},
//                         //         .value = HOMEKIT_INT_(100),
//                         //         .getter = led_speed_get,
//                         //         .setter = led_speed_set
//                         //         ),
//                         NULL
//                 }),
//                 NULL
//         }),
//         NULL
// };
//
// homekit_server_config_t config = {
//         .accessories = accessories,
//         .password = "111-11-111"
// };
//
void user_init(void) {
//         // This example shows how to use same firmware for multiple similar accessories
//         // without name conflicts. It uses the last 3 bytes of accessory's MAC address as
//         // accessory name suffix.
//         uint8_t macaddr[6];
//         sdk_wifi_get_macaddr(STATION_IF, macaddr);
//         int name_len = snprintf(NULL, 0, "Chihiro-%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
//         char *name_value = malloc(name_len + 1);
//         snprintf(name_value, name_len + 1, "Chihiro-%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
//         name.value = HOMEKIT_STRING(name_value);
//
//         wifi_init();
//
//         homekit_server_init(&config);
//
//         ws2812_init(LED_COUNT);
//
//         led_identify(HOMEKIT_INT(1));
}
