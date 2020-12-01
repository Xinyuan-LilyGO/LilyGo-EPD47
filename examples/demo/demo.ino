/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen.
 *
 * Write an image into a header file using a 3...2...1...0 format per pixel,
 * for 4 bits color (16 colors - well, greys.) MSB first.  At 80 MHz, screen
 * clears execute in 1.075 seconds and images are drawn in 1.531 seconds.
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"
#include "logo.h"
#include "firasans.h"

uint8_t *framebuffer;

void setup()
{
    Serial.begin(115200);

    // disable Core 0 WDT
    TaskHandle_t idle_0 = xTaskGetIdleTaskHandleForCPU(0);
    esp_task_wdt_delete(idle_0);

    epd_init();

    framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
}

void loop()
{
    delay(300);

    epd_poweron();
    volatile uint32_t t1 = millis();
    epd_clear();
    volatile uint32_t t2 = millis();
    printf("EPD clear took %dms.\n", t2 - t1);
    epd_poweroff();

    epd_draw_hline(20, 20, EPD_WIDTH - 40, 0x00, framebuffer);
    epd_draw_hline(20, EPD_HEIGHT - 20, EPD_WIDTH - 40, 0x00, framebuffer);
    epd_draw_vline(20, 20, EPD_HEIGHT - 40 + 1, 0x00, framebuffer);
    epd_draw_vline(EPD_WIDTH - 20, 20, EPD_HEIGHT - 40 + 1, 0x00, framebuffer);

    Rect_t area = {
        .x = 230,
        .y = 20,
        .width = logo_width,
        .height = logo_height,
    };
    epd_poweron();
    epd_draw_grayscale_image(area, (uint8_t *)logo_data);
    epd_poweroff();

    int cursor_x = 200;
    int cursor_y = 300;

    char *string1 = "➸ 16 color grayscale  😀 \n";
    char *string2 = "➸ Use with 4.7\" EPDs 😍 \n";
    char *string3 = "➸ High-quality font rendering ✎🙋";
    char *string4 = "➸ ~630ms for full frame draw 🚀\n";

    epd_poweron();
    writeln((GFXfont *)&FiraSans, string1, &cursor_x, &cursor_y, NULL);
    delay(500);
    cursor_x = 200;
    cursor_y += 50;
    writeln((GFXfont *)&FiraSans, string2, &cursor_x, &cursor_y, NULL);
    delay(500);
    cursor_x = 200;
    cursor_y += 50;
    writeln((GFXfont *)&FiraSans, string3, &cursor_x, &cursor_y, NULL);
    delay(500);
    cursor_x = 200;
    cursor_y += 50;
    writeln((GFXfont *)&FiraSans, string4, &cursor_x, &cursor_y, NULL);
    delay(500);

    epd_poweroff();
    delay(5000);



    // esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ALL_LOW);
    // esp_deep_sleep_start();

}
