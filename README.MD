<h1 align = "center">🌟LilyGo EPD47 🌟</h1>

## **English | [中文](./README_CN.MD)**

- The driver and sample program are from [vroland/epdiy](https://github.com/vroland/epdiy)

<h3 align = "left">Quick start:</h3>

- Install the [Arduino IDE](https://www.arduino.cc/en/Main/Software). Note: Later instructions may not work if you use Arduino via Flatpak.
- Install the latest [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) package in the IDE, version >= 1.05
- Download a zipfile from github using the "Download ZIP" button and install it using the IDE ("Sketch" -> "Include Library" -> "Add .ZIP Library...", OR:
- Clone this git repository into your sketchbook/libraries folder. For more info, see https://www.arduino.cc/en/Guide/Libraries
- Choose `ESP32 Dev Module` for the board
- Tools -> PSRAM -> `Enable`
- ("File" -> "Examples" -> "LilyGoEPD47" -> "demo") in the Arduino IDE
- Select the correct port and click upload
- Please refer to [Script Instructions](./scripts/README.MD) for picture and text generation 

# FAQ

- IO0 has been used and cannot be programmed.
- How to turn off the blue LED light?
    - To turn off the blue LED, you must call `epd_poweroff_all()`, which will turn off the entire power supply controlled by PWM_EN. Note that it cannot be turned off when powered by USB. It can only be turned off when powered by battery. For details, refer to `<examples/demo>`
