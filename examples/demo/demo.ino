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
#include <Wire.h>
#include "logo.h"

#define BATT_PIN            36


uint8_t *framebuffer;
int vref = 1100;

void setup()
{
    Serial.begin(115200);

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_0, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }

    epd_init();

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    Rect_t area = {
        .x = 230,
        .y = 20,
        .width = logo_width,
        .height = logo_height,
    };
    epd_poweron();
    epd_clear();
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

}

void loop()
{


    uint16_t v = analogRead(BATT_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    String voltage = "➸ Voltage :" + String(battery_voltage) + "V";
    Serial.println(voltage);

    Rect_t area = {
        .x = 200,
        .y = 460,
        .width = 320,
        .height = 50,
    };

    int cursor_x = 200;
    int cursor_y = 500;
    epd_poweron();
    epd_clear_area(area);
    writeln((GFXfont *)&FiraSans, (char *)voltage.c_str(), &cursor_x, &cursor_y, NULL);
    epd_poweroff();
    delay(5000);
}
