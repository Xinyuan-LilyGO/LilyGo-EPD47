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
#include "firasans.h"
#include "esp_adc_cal.h"
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include "logo.h"

#include <Wire.h>
#include <TouchDrvGT911.hpp>        
#include <SensorPCF8563.hpp>
#include <WiFi.h>
#include <esp_sntp.h>
#include "utilities.h"


#define WIFI_SSID             "Your WiFi SSID"
#define WIFI_PASSWORD         "Your WiFi PASSWORD"

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
const char *time_zone = "CST-8";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


SensorPCF8563 rtc;
TouchDrvGT911 touch;

uint8_t *framebuffer = NULL;
bool touchOnline = false;
uint32_t interval = 0;
int vref = 1100;
char buf[128];

struct _point {
    uint8_t buttonID;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
} touchPoint[] = {
    {0, 10, 10, 80, 80},
    {1, EPD_WIDTH - 80, 10, 80, 80},
    {2, 10, EPD_HEIGHT - 80, 80, 80},
    {3, EPD_WIDTH - 80, EPD_HEIGHT - 80, 80, 80},
    {4, EPD_WIDTH / 2 - 60, EPD_HEIGHT - 80, 120, 80}
};

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void timeavailable(struct timeval *t)
{
    Serial.println("[WiFi]: Got time adjustment from NTP!");
    rtc.hwClockWrite();
}

void setup()
{
    Serial.begin(115200);


    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // set notification call-back function
    sntp_set_time_sync_notification_cb( timeavailable );

    /**
     * This will set configured ntp servers and constant TimeZone/daylightOffset
     * should be OK if your time zone does not need to adjust daylightOffset twice a year,
     * in such a case time adjustment won't be handled automagicaly.
     */
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

    configTzTime(time_zone, ntpServer1, ntpServer2);


    /**
    * SD Card test
    * Only as a test SdCard hardware, use example reference
    * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD/examples
    */
    SPI.begin(SD_SCLK, SD_SCLK, SD_MOSI, SD_CS);
    bool rlst = SD.begin(SD_CS, SPI);
    if (!rlst) {
        Serial.println("SD init failed");
        snprintf(buf, 128, "➸ No detected SdCard 😂");
    } else {
        Serial.println("SD init success");
        snprintf(buf, 128,
                 "➸ Detected SdCard insert:%.2f GB😀",
                 SD.cardSize() / 1024.0 / 1024.0 / 1024.0
                );
    }

    // Correct the ADC reference voltage
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
                                       ADC_UNIT_2,
                                       ADC_ATTEN_DB_11,
                                       ADC_WIDTH_BIT_12,
                                       1100,
                                       &adc_chars
                                   );

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref: %umV\r\n", adc_chars.vref);
        vref = adc_chars.vref;
    }


    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    epd_init();

    Rect_t area = {
        .x = 230,
        .y = 0,
        .width = logo_width,
        .height = logo_height,
    };

    epd_poweron();
    epd_clear();
    epd_draw_grayscale_image(area, (uint8_t *)logo_data);
    epd_draw_image(area, (uint8_t *)logo_data, BLACK_ON_WHITE);


    int cursor_x = 200;
    int cursor_y = 200;


#if defined(CONFIG_IDF_TARGET_ESP32S3)
    // Assuming that the previous touch was in sleep state, wake it up
    pinMode(TOUCH_INT, OUTPUT);
    digitalWrite(TOUCH_INT, HIGH);


    writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);
    cursor_x = 200;
    cursor_y += 50;


    Wire.begin(BOARD_SDA, BOARD_SCL);
    Wire.beginTransmission(PCF8563_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        rtc.begin(Wire, PCF8563_SLAVE_ADDRESS, BOARD_SDA, BOARD_SCL);
        // rtc.setDateTime(2022, 6, 30, 0, 0, 0);
        writeln((GFXfont *)&FiraSans, "➸ RTC is online  😀 \n", &cursor_x, &cursor_y, NULL);
    } else {
        writeln((GFXfont *)&FiraSans, "➸ RTC is probe failed!  😂 \n", &cursor_x, &cursor_y, NULL);
    }


    /*
    * The touch reset pin uses hardware pull-up,
    * and the function of setting the I2C device address cannot be used.
    * Use scanning to obtain the touch device address.*/
    uint8_t touchAddress = 0x14;

    Wire.beginTransmission(0x14);
    if (Wire.endTransmission() == 0) {
        touchAddress = 0x14;
    }
    Wire.beginTransmission(0x5D);
    if (Wire.endTransmission() == 0) {
        touchAddress = 0x5D;
    }

    cursor_x = 200;
    cursor_y += 50;

    touch.setPins(-1, TOUCH_INT);
    if (touch.begin(Wire, touchAddress, BOARD_SDA, BOARD_SCL )) {
        touch.setMaxCoordinates(EPD_WIDTH, EPD_HEIGHT);
        touch.setSwapXY(true);
        touch.setMirrorXY(false, true);
        touchOnline = true;
        writeln((GFXfont *)&FiraSans, "➸ Touch is online  😀 \n", &cursor_x, &cursor_y, NULL);
    } else {
        writeln((GFXfont *)&FiraSans, "➸ Touch is probe failed!  😂 \n", &cursor_x, &cursor_y, NULL);
    }

#endif

    FontProperties props = {
        .fg_color = 15,
        .bg_color = 0,
        .fallback_glyph = 0,
        .flags = 0
    };

    // Draw button
    int32_t x = 18;
    int32_t y = 50;
    epd_fill_rect(10, 10, 80, 80, 0x0000, framebuffer);
    write_mode((GFXfont *)&FiraSans, "A", &x, &y, framebuffer, WHITE_ON_BLACK, &props);

    x = EPD_WIDTH - 72;
    y = 50;
    epd_fill_rect(EPD_WIDTH - 80, 10, 80, 80, 0x0000, framebuffer);
    write_mode((GFXfont *)&FiraSans, "B", &x, &y, framebuffer, WHITE_ON_BLACK, &props);

    x = 18;
    y = EPD_HEIGHT - 30;
    epd_fill_rect(10, EPD_HEIGHT - 80, 80, 80, 0x0000, framebuffer);
    write_mode((GFXfont *)&FiraSans, "C", &x, &y, framebuffer, WHITE_ON_BLACK, &props);

    x = EPD_WIDTH - 72;
    y = EPD_HEIGHT - 30;
    epd_fill_rect(EPD_WIDTH - 80, EPD_HEIGHT - 80, 80, 80, 0x0000, framebuffer);
    write_mode((GFXfont *)&FiraSans, "D", &x, &y, framebuffer, WHITE_ON_BLACK, &props);


    x = EPD_WIDTH / 2 - 55;
    y = EPD_HEIGHT - 30;
    epd_draw_rect(EPD_WIDTH / 2 - 60, EPD_HEIGHT - 80, 120, 75, 0x0000, framebuffer);
    write_mode((GFXfont *)&FiraSans, "Sleep", &x, &y, framebuffer, WHITE_ON_BLACK, NULL);

    epd_draw_grayscale_image(epd_full_screen(), framebuffer);


    epd_poweroff();

}


void loop()
{

    if (millis() > interval) {
        interval = millis() + 10000;

        // When reading the battery voltage, POWER_EN must be turned on
        epd_poweron();
        delay(10); // Make adc measurement more accurate
        uint16_t v = analogRead(BATT_PIN);
        float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
        if (battery_voltage >= 4.2) {
            battery_voltage = 4.2;
        }
        String voltage = "➸ Voltage: " + String(battery_voltage) + "V";

        Rect_t area = {
            .x = 200,
            .y = 310,
            .width = 500,
            .height = 100,
        };

        int cursor_x = 200;
        int cursor_y = 350;
        epd_clear_area(area);

        writeln((GFXfont *)&FiraSans, (char *)voltage.c_str(), &cursor_x, &cursor_y, NULL);
        cursor_x = 200;
        cursor_y += 50;

        // Format the output using the strftime function
        // For more formats, please refer to :
        // https://man7.org/linux/man-pages/man3/strftime.3.html

        struct tm timeinfo;
        // Get the time C library structure
        rtc.getDateTime(&timeinfo);

        strftime(buf, 64, "➸ %b %d %Y %H:%M:%S", &timeinfo);
        writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);

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
    }


    if (touchOnline) {
        int16_t  x, y;

        if (!digitalRead(TOUCH_INT)) {
            return;
        }

        uint8_t touched = touch.getPoint(&x, &y);
        if (touched) {

            // When reading the battery voltage, POWER_EN must be turned on
            epd_poweron();

            int cursor_x = 200;
            int cursor_y = 450;

            Rect_t area = {
                .x = 200,
                .y = 410,
                .width = 400,
                .height = 50,
            };
            epd_clear_area(area);

            snprintf(buf, 128, "➸ X:%d Y:%d", x, y);

            bool pressButton = false;
            for (int i = 0; i < sizeof(touchPoint) / sizeof(touchPoint[0]); ++i) {
                if ((x > touchPoint[i].x && x < (touchPoint[i].x + touchPoint[i].w))
                        && (y > touchPoint[i].y && y < (touchPoint[i].y + touchPoint[i].h))) {
                    snprintf(buf, 128, "➸ Pressed Button: %c\n", 65 + touchPoint[i].buttonID);
                    writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);
                    pressButton = true;

                    if ( touchPoint[i].buttonID == 4) {

                        Serial.println("Sleep !!!!!!");

                        epd_clear();

                        cursor_x = EPD_WIDTH / 2 - 40;
                        cursor_y = EPD_HEIGHT / 2 - 40;

                        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

                        writeln((GFXfont *)&FiraSans, "Sleep", &cursor_x, &cursor_y, framebuffer);

                        epd_draw_grayscale_image(epd_full_screen(), framebuffer);

                        delay(1000);

                        epd_poweroff_all();

                        WiFi.disconnect(true);

                        touch.sleep();

                        delay(100);


                        Wire.end();

                        Serial.end();

                        // BOOT(STR_IO0) Button wakeup
                        esp_sleep_enable_ext1_wakeup(_BV(0), ESP_EXT1_WAKEUP_ANY_LOW);

                        esp_deep_sleep_start();

                    }

                }
            }
            if (!pressButton) {
                writeln((GFXfont *)&FiraSans, buf, &cursor_x, &cursor_y, NULL);
            }

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
        }
    }

    delay(2);
}
