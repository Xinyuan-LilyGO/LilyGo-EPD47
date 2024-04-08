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
#include <Arduino.h>
#include "epd_driver.h"
#include "utilities.h"

void setup()
{
    int32_t i = 0;

    epd_init();

    Rect_t area = epd_full_screen();
    epd_poweron();
    delay(10);
    epd_clear();
    for (i = 0; i < 20; i++)
    {
        epd_push_pixels(area, 50, 0);
        delay(500);
    }
    epd_clear();
    for (i = 0; i < 40; i++)
    {
        epd_push_pixels(area, 50, 1);
        delay(500);
    }
    epd_clear();
    epd_poweroff_all();
}

void loop()
{
}
