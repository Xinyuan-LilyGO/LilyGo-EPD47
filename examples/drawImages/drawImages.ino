/**
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-04-05
 * @note      Arduino Setting
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  USB DFU On Boot:"Disable"
 *                  Flash Size : "16MB(128Mb)"
 *                  Flash Mode"QIO 80MHz
 *                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 *                  PSRAM:"OPI PSRAM"
 *                  Upload Mode:"UART0/Hardware CDC"
 *                  USB Mode:"Hardware CDC and JTAG"
 *  
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen.
 *
 * Write an image into a header file using a 3...2...1...0 format per pixel,
 * for 4 bits color (16 colors - well, greys.) MSB first.  At 80 MHz, screen
 * clears execute in 1.075 seconds and images are drawn in 1.531 seconds.
 */

#include <Arduino.h>
#include "epd_driver.h"
#include "src/pic1.h"
#include "src/pic2.h"
#include "src/pic3.h"
#include "utilities.h"

uint8_t *framebuffer;

void setup()
{
    Serial.begin(115200);

    epd_init();

    framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);

    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1)
            ;
    }

    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
}

void update(uint32_t delay_ms)
{
    epd_poweron();
    epd_clear();
    volatile uint32_t t1 = millis();
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    volatile uint32_t t2 = millis();
    Serial.printf("EPD draw took %dms.\r\n", t2 - t1);
    epd_poweroff();
    delay(delay_ms);
}

void loop()
{
    Rect_t area = {
        .x = 0,
        .y = 0,
        .width = pic1_width,
        .height = pic1_height
    };
    epd_poweron();
    epd_clear();
    epd_draw_grayscale_image(area, (uint8_t *)pic1_data);
    epd_poweroff();
    delay(3000);

    Rect_t area1 = {
        .x = 0,
        .y = 0,
        .width = pic2_width,
        .height = pic2_height
    };
    epd_copy_to_framebuffer(area1, (uint8_t *)pic2_data, framebuffer);

    update(3000);

    Rect_t area2 = {
        .x = 0,
        .y = 0,
        .width = pic3_width,
        .height = pic3_height
    };
    epd_copy_to_framebuffer(area2, (uint8_t *)pic3_data, framebuffer);

    update(3000);
}
