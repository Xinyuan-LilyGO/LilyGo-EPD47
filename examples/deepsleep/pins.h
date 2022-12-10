#ifndef PINS_H_
#define PINS_H_

#if defined(CONFIG_IDF_TARGET_ESP32)
#define T5_47
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define T5_47_PLUS
#else
#error "Unsupported board"
#endif

#if defined(T5_47)
#define BUTTON_1  (34)
#define BUTTON_2  (35)
#define BUTTON_3  (39)

#define BATT_PIN  (36)

#define SD_MISO   (12)
#define SD_MOSI   (13)
#define SD_SCLK   (14)
#define SD_CS     (15)

#define TOUCH_SCL (14)
#define TOUCH_SDA (15)
#define TOUCH_INT (13)

#define GPIO_MISO (12)
#define GPIO_MOSI (13)
#define GPIO_SCLK (14)
#define GPIO_CS   (15)

#define BUTTON_PIN_BITMASK GPIO_SEL_34

#elif defined(T5_47_PLUS)
#define BUTTON_1   (21)

#define BATT_PIN   (14)

#define SD_MISO    (16)
#define SD_MOSI    (15)
#define SD_SCLK    (11)
#define SD_CS      (42)

#define TOUCH_SCL  (17)
#define TOUCH_SDA  (18)
#define TOUCH_INT  (47)

#define GPIO_MISO  (45)
#define GPIO_MOSI  (10)
#define GPIO_SCLK  (48)
#define GPIO_CS    (39)

#define BUTTON_PIN_BITMASK GPIO_SEL_0
#endif

#endif