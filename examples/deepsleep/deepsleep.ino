#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include "epd_driver.h"
#include "font/firasans.h"
#include "esp_adc_cal.h"
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include "image/logo.h"
#include <touch.h>
#include "pins.h"

// #define USING_TOUCH_PANEL

#if defined(T5_47_PLUS)
#include "pcf8563.h"
#include <Wire.h>
#endif

#if defined(T5_47_PLUS)
PCF8563_Class rtc;
#endif

static int vref = 1100;
TouchClass touch;

void setup() {
    Serial.begin(115200);
    delay(1000);

    print_wakeup_reason();

    char buf[128];
    /**
    * SD Card test
    * Only as a test SdCard hardware, use example reference
    * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD/examples
    */
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    // SD_MMC.setPins(SD_SCLK, SD_MOSI, SD_MISO);
    // bool rlst = SD_MMC.begin("/sdcard", true);
    bool rlst = SD.begin(SD_CS, SPI);
    if (!rlst) {
        Serial.println("SD init failed");
        snprintf(buf, 128, "➸ No detected SdCard");
    } else {
        Serial.println("SD init success");
        snprintf(buf, 128,
            "➸ Detected SdCard insert:%.2f GB",
            SD.cardSize() / 1024.0 / 1024.0 / 1024.0
        );
    }

    /** Correct the ADC reference voltage */
    esp_adc_cal_characteristics_t adc_chars;
#if defined(T5_47)
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        1100,
        &adc_chars
    );
#else
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_2,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        1100,
        &adc_chars
    );
#endif
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref: %umV\r\n", adc_chars.vref);
        vref = adc_chars.vref;
    }

    Wire.begin(TOUCH_SDA, TOUCH_SCL);

#if defined(CONFIG_IDF_TARGET_ESP32S3)
    rtc.begin();
    rtc.setDateTime(2022, 6, 30, 0, 0, 0);
#endif

#if defined(USING_TOUCH_PANEL)
    if (!touch.begin()) {
        Serial.println("start touchscreen failed");
    } else {
        Serial.println("touchscreen sleep");
        touch.sleep();
    }
#endif

    epd_init();

    Rect_t area = {
        .x = 230,
        .y = 20,
        .width = logo_width,
        .height = logo_height,
    };

    epd_poweron();
    epd_clear();
    // epd_draw_grayscale_image(area, (uint8_t *)logo_data);
    epd_draw_image(area, (uint8_t *)logo_data, BLACK_ON_WHITE);
    epd_poweroff();


    int cursor_x = 200;
    int cursor_y = 250;

    const char *string1 = "➸ 16 color grayscale  😀 \n";
    const char *string2 = "➸ Use with 4.7\" EPDs 😍 \n";
    const char *string3 = "➸ High-quality font rendering ✎🙋";
    const char *string4 = "➸ ~630ms for full frame draw 🚀\n";

    epd_poweron();

    writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);
    delay(500);
    cursor_x = 200;
    cursor_y += 50;
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


void loop() {
    /** When reading the battery voltage, POWER_EN must be turned on */
    epd_poweron();
    delay(10); /** Make adc measurement more accurate */
    uint16_t v = analogRead(BATT_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    String voltage = "➸ Voltage: " + String(battery_voltage) + "V";
#if defined(T5_47_PLUS)
    voltage = voltage + String(" (") + rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S) + String(")");
#endif
    Serial.println(voltage);

    Rect_t area = {
        .x = 200,
        .y = 460,
#if defined(T5_47_PLUS)
        .width = 700,
#else
        .width = 320,
#endif
        .height = 50,
    };

    int cursor_x = 200;
    int cursor_y = 500;
    epd_clear_area(area);
    writeln((GFXfont *)&FiraSans, (char *)voltage.c_str(), &cursor_x, &cursor_y, NULL);

    /**
     * There are two ways to close
     * It will turn off the power of the ink screen,
     * but cannot turn off the blue LED light.
     */
    // epd_poweroff();

    /**
     * It will turn off the power of the entire
     * POWER_EN control and also turn off the blue LED light
     */
    epd_poweroff_all();

    /**
    First we configure the wake up source
    We set our ESP32 to wake up for an external trigger.
    There are two types for ESP32, ext0 and ext1 .
    ext0 uses RTC_IO to wakeup thus requires RTC peripherals
    to be on while ext1 uses RTC Controller so doesnt need
    peripherals to be powered on.
    Note that using internal pullups/pulldowns also requires
    RTC peripherals to be turned on.
    */
    // esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, LOW); //1 = High, 0 = Low

    //If you were to use ext1, you would use it like
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);

    //Go to sleep now
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}


static void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wakeup caused by external signal using RTC_IO");
        break;

        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;

        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup caused by timer");
        break;

        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("Wakeup caused by touchpad");
        break;

        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("Wakeup caused by ULP program");
        break;

        default:
            Serial.printf(
                "Wakeup was not caused by deep sleep: %d\r\n",
                wakeup_reason
            );
        break;
    }
}
