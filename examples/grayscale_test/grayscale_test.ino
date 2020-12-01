/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen.
 *
 * Write an image into a header file using a 3...2...1...0 format per pixel,
 * for 4 bits color (16 colors - well, greys.) MSB first.  At 80 MHz, screen
 * clears execute in 1.075 seconds and images are drawn in 1.531 seconds.
 */

#include <Arduino.h>
#include "epd_driver.h"

uint8_t *grayscale_img;
uint8_t *grayscale_img2;

void setup()
{
    Serial.begin(115200);

    epd_init();

    // copy the image data to SRAM for faster display time
    grayscale_img = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    if (grayscale_img == NULL) {
        Serial.println( "Could not allocate framebuffer in PSRAM!");
    }

    grayscale_img2 = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    if (grayscale_img2 == NULL) {
        Serial.println( "Could not allocate framebuffer in PSRAM!");
    }

    uint8_t grayscale_line[EPD_WIDTH / 2];
    uint8_t value = 0;
    for (uint32_t i = 0; i < EPD_WIDTH / 2; i++) {
        uint8_t segment = i / (EPD_WIDTH / 16 / 2);
        grayscale_line[i] = (segment << 4) | segment;
    }
    for (uint32_t y = 0; y < EPD_HEIGHT; y++) {
        memcpy(grayscale_img + EPD_WIDTH / 2 * y, grayscale_line, EPD_WIDTH / 2);
    }

    value = 0;
    for (uint32_t i = 0; i < EPD_WIDTH / 2; i++) {
        uint8_t segment = (EPD_WIDTH / 2 - i - 1) / (EPD_WIDTH / 16 / 2);
        grayscale_line[i] = (segment << 4) | segment;
    }
    for (uint32_t y = 0; y < EPD_HEIGHT; y++) {
        memcpy(grayscale_img2 + EPD_WIDTH / 2 * y, grayscale_line, EPD_WIDTH / 2);
    }
}

void loop()
{
    uint32_t t1, t2;
    Serial.println("WHITE_ON_BLACK drawing:");
    epd_poweron();
    epd_clear();
    t1 = esp_timer_get_time();
    epd_push_pixels(epd_full_screen(), 20, 0);
    epd_push_pixels(epd_full_screen(), 20, 0);
    epd_push_pixels(epd_full_screen(), 20, 0);
    epd_draw_image(epd_full_screen(), grayscale_img2, WHITE_ON_BLACK);
    t2 = esp_timer_get_time();
    Serial.printf("draw took %dms.\n", (t2 - t1) / 1000);
    epd_poweroff();

    delay(3000);

    Serial.println("BLACK_ON_WHITE drawing:");
    epd_poweron();
    epd_clear();
    t1 = esp_timer_get_time();
    epd_draw_image(epd_full_screen(), grayscale_img, BLACK_ON_WHITE);
    t2 = esp_timer_get_time();
    Serial.printf("draw took %dms.\n", (t2 - t1) / 1000);
    epd_poweroff();
    delay(3000);

}
