#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include "epd_driver.h"

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
