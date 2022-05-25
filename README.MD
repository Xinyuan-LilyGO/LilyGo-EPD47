<h1 align = "center">🌟LilyGo EPD47 🌟</h1>

## **English | [中文](./README_CN.MD)**

- The driver and sample program are from [vroland/epdiy](https://github.com/vroland/epdiy)

## Quick Start

- Install the [Arduino IDE](https://www.arduino.cc/en/Main/Software). Note: Later instructions may not work if you use Arduino via Flatpak.
- Install the latest [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) package in the IDE, version >= 1.05
- Download a zipfile from github using the "Download ZIP" button and install it using the IDE ("Sketch" -> "Include Library" -> "Add .ZIP Library...", OR:
- Clone this git repository into your sketchbook/libraries folder. For more info, see https://www.arduino.cc/en/Guide/Libraries
- Choose `ESP32 Dev Module` for the board
- Tools -> PSRAM -> `Enable`
- ("File" -> "Examples" -> "LilyGoEPD47" -> "demo") in the Arduino IDE
- Select the correct port and click upload
- Please refer to [Script Instructions](./scripts/README.MD) for picture and text generation 

## Related Projects

- [OpenWeather Display](https://github.com/Xinyuan-LilyGO/LilyGo-EPD-4-7-OWM-Weather-Display.git)
- [EPD47-HC08](https://github.com/Xinyuan-LilyGO/EPD47-HC08.git)
- [Variometer](https://github.com/Oganisyan/Variometer-LilyGoEPD47)

## FAQ

1. Which GPIOs are left for free use?

    IO12.
    If the touch screen is not used, there are also IO12, IO13, IO14 and IO15 which can be used freely.
    IO0 has been used and cannot be programmed.

2. How to turn off the blue LED light?

    To turn off the blue LED, you must call `epd_poweroff_all()`, which will turn off the entire power supply controlled by PWM_EN. Note that it cannot be turned off when powered by USB. It can only be turned off when powered by battery. For details, refer to [examples/demo](./examples/demo)

3. How to fix afterimages?

    Please upload [examples/screen_repair](./examples/screen_repair) to try to remove afterimages.
    It is not 100% able to remove afterimages.