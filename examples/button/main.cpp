#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"
#include "firasans.h"
#include "esp_adc_cal.h"
#include "Button2.h"
#include <Wire.h>
#include "lilygo.h"
#include "logo.h"

#define BATT_PIN            36
#define BUTTON_1            34
#define BUTTON_2            35
#define BUTTON_3            39


Button2  btn1(BUTTON_1);
Button2  btn2(BUTTON_2);
Button2  btn3(BUTTON_3);

uint8_t *framebuffer;
int vref = 1100;
int cursor_x = 20;
int cursor_y = 60;
int state = 0;

Rect_t area1 = {
    .x = 10,
    .y = 20,
    .width = EPD_WIDTH - 20,
    .height =  EPD_HEIGHT / 2 + 80
};

const char *overview[] = {
    "   ESP32 is a single 2.4 GHz Wi-Fi-and-Bluetooth\n"\
    "combo chip designed with the TSMC ultra-low-po\n"\
    "wer 40 nm technology. It is designed to achieve \n"\
    "the best power and RF performance, showing rob\n"\
    "ustness versatility and reliability in a wide variet\n"\
    "y of applications and power scenarios.\n",
    "➸ Xtensa® dual-core 32-bit LX6 microprocessor\n"\
    "➸ 448 KB ROM & External 16MBytes falsh\n"\
    "➸ 520 KB SRAM & External 16MBytes PSRAM\n"\
    "➸ 16 KB SRAM in RTC\n"\
    "➸ Multi-connections in Classic BT and BLE\n"\
    "➸ 802.11 n (2.4 GHz), up to 150 Mbps\n",
    "➸ 16 color grayscale\n"\
    "➸ Use with 4.7\" EPDs\n"\
    "➸ High-quality font rendering\n"\
    "➸ ~630ms for full frame draw\n"
};


void displayInfo(void)
{
    cursor_x = 20;
    cursor_y = 60;
    state %= 4;
    switch (state) {
    case 0:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[0], &cursor_x, &cursor_y, NULL);
        break;
    case 1:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[1], &cursor_x, &cursor_y, NULL);
        break;
    case 2:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[2], &cursor_x, &cursor_y, NULL);
        break;
    case 3:
        delay(1000);
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, "DeepSleep", &cursor_x, &cursor_y, NULL);
        epd_poweroff_all();
        // Set to wake up by GPIO39
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ALL_LOW);
        esp_deep_sleep_start();
        break;
    case 4:
        break;
    default:
        break;
    }
    epd_poweroff();
}


void buttonPressed(Button2 &b)
{
    displayInfo();
    state++;
}


void setup()
{
    Serial.begin(115200);

    epd_init();

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    btn1.setPressedHandler(buttonPressed);
    btn2.setPressedHandler(buttonPressed);
    btn3.setPressedHandler(buttonPressed);

    epd_poweron();
    epd_clear();
    write_string((GFXfont *)&FiraSans, (char *)overview[0], &cursor_x, &cursor_y, framebuffer);

    //Draw Box
    epd_draw_rect(600, 450, 120, 60, 0, framebuffer);
    cursor_x = 615;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Prev", &cursor_x, &cursor_y, framebuffer);

    epd_draw_rect(740, 450, 120, 60, 0, framebuffer);
    cursor_x = 755;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Next", &cursor_x, &cursor_y, framebuffer);

    Rect_t area = {
        .x = 160,
        .y = 420,
        .width = lilygo_width,
        .height =  lilygo_height
    };
    epd_copy_to_framebuffer(area, (uint8_t *) lilygo_data, framebuffer);

    epd_draw_rect(10, 20, EPD_WIDTH - 20, EPD_HEIGHT / 2 + 80, 0, framebuffer);

    epd_draw_grayscale_image(epd_full_screen(), framebuffer);

    epd_poweroff();

}

void loop()
{
    btn1.loop();
    btn2.loop();
    btn3.loop();
}
