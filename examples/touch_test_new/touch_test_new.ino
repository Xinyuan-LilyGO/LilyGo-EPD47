/**
 * @note
 * * The touch is only suitable for the GT911 driver chip . 
 */
#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"
#include "image/logo.h"
#include "font/firasans.h"
#include <Wire.h>
#include "image/lilygo.h"
#include "pins.h"
#include <TouchDrvGT911.hpp>

TouchDrvGT911 touch;
uint8_t *framebuffer = NULL;

const char overview[] = {
    "   ESP32 is a single 2.4 GHz Wi-Fi-and-Bluetooth\n"\
    "combo chip designed with the TSMC ultra-low-po\n"\
    "wer 40 nm technology. It is designed to achieve \n"\
    "the best power and RF performance, showing rob\n"\
    "ustness versatility and reliability in a wide variet\n"\
    "y of applications and power scenarios.\n"\
};

const char mcu_features[] = {
    "➸ Xtensa® dual-core 32-bit LX6 microprocessor\n"\
    "➸ 448 KB ROM & External 16MBytes falsh\n"\
    "➸ 520 KB SRAM & External 16MBytes PSRAM\n"\
    "➸ 16 KB SRAM in RTC\n"\
    "➸ Multi-connections in Classic BT and BLE\n"\
    "➸ 802.11 n (2.4 GHz), up to 150 Mbps\n"\
};

const char srceen_features[] = {
    "➸ 16 color grayscale\n"\
    "➸ Use with 4.7\" EPDs\n"\
    "➸ High-quality font rendering\n"\
    "➸ ~630ms for full frame draw\n"\
};


// const char *string_array[] = {overview, mcu_features, srceen_features};

int cursor_x = 20;
int cursor_y = 60;

Rect_t area1 = {
    .x = 10,
    .y = 20,
    .width = EPD_WIDTH - 20,
    .height =  EPD_HEIGHT / 2 + 80
};
uint8_t state = 1;

void setup()
{
    Serial.begin(115200);

    epd_init();

    //* Sleep wakeup must wait one second, otherwise the touch device cannot be addressed
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
        delay(1000);
    }

    Wire.begin(TOUCH_SDA, TOUCH_SCL);

    /*
    * The touch reset pin uses hardware pull-up, 
    * and the function of setting the I2C device address cannot be used. 
    * Use scanning to obtain the touch device address.*/
    uint8_t touchAddress = 0;
    Wire.beginTransmission(0x14);
    if (Wire.endTransmission() == 0) {
        touchAddress = 0x14;
    }
    Wire.beginTransmission(0x5D);
    if (Wire.endTransmission() == 0) {
        touchAddress = 0x5D;
    }
    if (touchAddress == 0) {
        while (1) {
            Serial.println("Failed to find GT911 - check your wiring!");
            delay(1000);
        }
    }
    touch.setPins(-1, TOUCH_INT);
    if (!touch.init(Wire,  TOUCH_SDA, TOUCH_SCL, touchAddress )) {
        while (1) {
            Serial.println("Failed to find GT911 - check your wiring!");
            delay(1000);
        }
    }
    touch.setMaxCoordinates(EPD_WIDTH, EPD_HEIGHT);
    touch.setSwapXY(true);
    touch.setMirrorXY(false, true);

    Serial.println("Started Touchscreen poll...");

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    epd_poweron();
    epd_clear();
    write_string((GFXfont *)&FiraSans, (char *)overview, &cursor_x, &cursor_y, framebuffer);

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


int16_t  x, y;

void loop()
{
    uint8_t touched = touch.getPoint(&x, &y);
    if (touched) {
        // Serial.printf("X:%d Y:%d\n", x, y);
        if ((x > 600 && x < 720) && (y > 450 && y < 510)) {
            state--;
        } else if ((x > 740 && x < 860) && (y > 450 && y < 510)) {
            state++;
        } else {
            return;
        }
        state %= 4;
        Serial.print(millis());
        Serial.print(":");
        Serial.println(state);
        epd_poweron();
        cursor_x = 20;
        cursor_y = 60;
        switch (state) {
        case 0:
            epd_clear_area(area1);
            write_string((GFXfont *)&FiraSans, (char *)overview, &cursor_x, &cursor_y, NULL);
            break;
        case 1:
            epd_clear_area(area1);
            write_string((GFXfont *)&FiraSans, (char *)srceen_features, &cursor_x, &cursor_y, NULL);
            break;
        case 2:
            epd_clear_area(area1);
            write_string((GFXfont *)&FiraSans, (char *)mcu_features, &cursor_x, &cursor_y, NULL);
            break;
        case 3:
            delay(1000);
            epd_clear_area(area1);
            write_string((GFXfont *)&FiraSans, "DeepSleep", &cursor_x, &cursor_y, NULL);

            //* After calling touch sleep, you cannot wake it up by calling wakeup. You must turn the power off and on again to enable it.
            touch.sleep();
            delay(5);

            pinMode(14, INPUT);
            pinMode(15, INPUT);

            epd_poweroff_all();

            // esp_sleep_enable_ext1_wakeup(GPIO_SEL_13, ESP_EXT1_WAKEUP_ANY_HIGH);

#if defined(T5_47)
            // Set to wake up by GPIO39
            esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ANY_LOW);
#elif defined(T5_47_PLUS)
            esp_sleep_enable_ext1_wakeup(GPIO_SEL_21, ESP_EXT1_WAKEUP_ANY_LOW);
#endif
            esp_deep_sleep_start();
            break;
        case 4:
            break;
        default:
            break;
        }
        epd_poweroff();
    }
    delay(10);
}
