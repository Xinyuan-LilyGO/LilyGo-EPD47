<h1 align = "center">🌟LilyGo EPD47 🌟</h1>

<!-- ## **English | [中文](./README_CN.MD)** -->

## News

- The driver and sample program are from [vroland/epdiy](https://github.com/vroland/epdiy)

## 1️⃣ Support Product

| Product(PinMap)   | SOC        | Flash | PSRAM    | Resolution | Size     |
| ----------------- | ---------- | ----- | -------- | ---------- | -------- |
| [T5-ePaper-S3][1] | ESP32-S3R8 | 16MB  | 8MB(OPI) | 960x540    | 4.7 Inch |

| Current consumption | Working current             | sleep current | sleep mode    |
| ------------------- | --------------------------- | ------------- | ------------- |
| [T5-ePaper-S3][1]   | (240MHz) WiFi On 90~230+ mA | About 380uA   | button wakeup |

[1]: https://www.lilygo.cc/products/t5-4-7-inch-e-paper-v2-3

## 2️⃣ Examples

```txt
examples/
├── button              ; Keystroke example
├── demo                ; Comprehensive test example including sleep current test
├── drawExample         ; Simple examples of drawing lines and circles
├── drawImages          ; Show image example
├── grayscale_test      ; Grayscale example
├── screen_repair       ; Full screen refresh example
├── spi_driver          ; Display as slave device
├── touch               ; Touch example
└── wifi_sync           ; WiFi Comprehensive Example
```

## 3️⃣ PlatformIO Quick Start (recommended)

1. Install [Visual Studio Code](https://code.visualstudio.com/) and [Python](https://www.python.org/)
2. Search for the `PlatformIO` plugin in the `VisualStudioCode` extension and install it.
3. After the installation is complete, you need to restart `VisualStudioCode`
4. After restarting `VisualStudioCode`, select `File` in the upper left corner of `VisualStudioCode` -> `Open Folder` -> select the `LilyGo-EPD47` directory
5. Wait for the installation of third-party dependent libraries to complete
6. Click on the `platformio.ini` file, and in the `platformio` column
7. Uncomment one of the lines `src_dir = xxxx` to make sure only one line works
8. Click the (✔) symbol in the lower left corner to compile
9. Connect the board to the computer USB
10. Click (→) to upload firmware
11. Click (plug symbol) to monitor serial output
12. If it cannot be written, or the USB device keeps flashing, please check the **FAQ** below

## 4️⃣ Install from Arduino Library Manager (recommended)

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install [Arduino ESP32 V 2.0.5 or above and below V3.0](https://docs.espressif.com/projects/arduino-esp32/en/latest/) ，recommended to use version 2.0.15, the URL is as follows
    Steps: Arduino IDE -> Preferences -> Additional boards manager URLs
    ```url
    https://espressif.github.io/arduino-esp32/package_esp32_index.json
    ```
3. `Sketch` -> `Include Library` -> `Manage Libraries`
4. `Library Search` -> `LilyGo EPD47` -> `Install` -> `Install ALL`
5. `File` -> `Examples` -> `LilyGo EPD47` -> `Any Examples`
6. `Tools` , Look at the picture to choose
   | Arduino IDE Setting                  | Value                             |
   | ------------------------------------ | --------------------------------- |
   | Board                                | **ESP32S3 Dev Module**            |
   | Port                                 | Your port                         |
   | USB CDC On Boot                      | Enable                            |
   | CPU Frequency                        | 240MHZ(WiFi)                      |
   | Core Debug Level                     | None                              |
   | USB DFU On Boot                      | Disable                           |
   | Erase All Flash Before Sketch Upload | Disable                           |
   | Events Run On                        | Core1                             |
   | Flash Mode                           | QIO 80MHZ                         |
   | Flash Size                           | **16MB(128Mb)**                   |
   | Arduino Runs On                      | Core1                             |
   | USB Firmware MSC On Boot             | Disable                           |
   | Partition Scheme                     | **16M Flash(3M APP/9.9MB FATFS)** |
   | PSRAM                                | **OPI PSRAM**                     |
   | Upload Mode                          | **UART0/Hardware CDC**            |
   | Upload Speed                         | 921600                            |
   | USB Mode                             | **CDC and JTAG**                  |

   - The options in bold are required, others are selected according to actual conditions.

7. Select `Port`
8. Click `upload` , Wait for compilation and writing to complete
9.  If it cannot be written, or the USB device keeps flashing, please check the **FAQ** below

## 5️⃣ Install from Arduino CLI (recommended)

1. Install the [Arduino CLI](https://arduino.github.io/arduino-cli/latest/installation)
2. Run these commands to set up the environment:

    ```shell
    arduino-cli config init
    arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
    arduino-cli core update-index
    arduino-cli core install esp32:esp32@2.0.17
    arduino-cli lib install "SensorLib@0.3.1"
    arduino-cli lib install "Button2@2.3.2"
    arduino-cli lib install "ArduinoJson@7.4.2"
    arduino-cli lib install LilyGo-EPD47
    ```

3. Run this command to compile your sketch:

    ```shell
    arduino-cli compile -besp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi
    ```

## 6️⃣ Related Projects

- [OpenWeather Display](https://github.com/Xinyuan-LilyGO/LilyGo-EPD-4-7-OWM-Weather-Display/tree/web)
- [EPD47-HC08](https://github.com/Xinyuan-LilyGO/EPD47-HC08.git)
- [Variometer](https://github.com/Oganisyan/Variometer-LilyGoEPD47)


## 7️⃣ ESP32 basic examples

- [BLE Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE)
- [WiFi Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
- [SPIFFS Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS)
- [FFat Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/FFat)
- For more examples of esp32 chip functions, please refer to [arduino-esp32-libraries](https://github.com/espressif/arduino-esp32/tree/master/libraries)

## 8️⃣ FAQ

1. Is there no voltage at the Molex connector?
    * The voltage for the Molex connector is controlled by Pin 13 of the **74HCT4094** (schematic reference QP5); software must call `epd_poweron()` to enable it. Since the Molex connector shares the same power supply as the screen, the Molex voltage is cut off when the screen power is turned off; `epd_poweroff()` shuts down power to both the screen and the Molex connector.
    * Molex connector and 3.3V Pin Header no more than 100mA load

2. Which one does the sleep current measurement example use? ?

    Sleep current test measured by [demo](./examples/demo/demo.ino) example

3. How to fix afterimages?

    Please upload [examples/screen_repair](./examples/screen_repair) to try to remove afterimages.
    It is not 100% able to remove afterimages.

4. The board uses USB as the JTAG upload port. When printing serial port information on USB_CDC_ON_BOOT configuration needs to be turned on.
If the port cannot be found when uploading the program or the USB has been used for other functions, the port does not appear.
Please enter the upload mode manually.

   ![product](images/product.jpg)

   1. Connect the board via the USB cable
   2. Press and hold the BOOT(IO0) button , While still pressing the BOOT(IO0) button, press RST
   3. Release the RST
   4. Release the BOOT(IO0) button
   5. Upload sketch

## 9️⃣ GPIO List

| ESP32S3 GPIO | Connect To            | Free |
| ------------ | --------------------- | ---- |
| 13           | 74HCT4094D CFG_DATA   | ❌    |
| 12           | 74HCT4094D CFG_CLK    | ❌    |
| 0            | 74HCT4094D CFG_STR    | ❌    |
| 38           | E-paper CKV           | ❌    |
| 40           | E-paper STH           | ❌    |
| 41           | E-paper CKH           | ❌    |
| 8            | E-paper D0            | ❌    |
| 1            | E-paper D1            | ❌    |
| 2            | E-paper D2            | ❌    |
| 3            | E-paper D3            | ❌    |
| 4            | E-paper D4            | ❌    |
| 5            | E-paper D5            | ❌    |
| 6            | E-paper D6            | ❌    |
| 7            | E-paper D7            | ❌    |
| 21           | Button                | ❌    |
| 14           | Battery ADC           | ❌    |
| 16           | SD MISO               | ❌*   |
| 15           | SD MOSI               | ❌*   |
| 11           | SD  SCK               | ❌*   |
| 42           | SD  CS                | ❌*   |
| 18           | RTC (Touchscreen) SDA | ❌    |
| 17           | RTC (Touchscreen) SCL | ❌    |
| 47           | TouchPanel IRQ        | ❌    |
| 45           | No Connect any        | ✅    |
| 10           | No Connect any        | ✅    |
| 48           | No Connect any        | ✅    |
| 39           | No Connect any        | ✅    |

- The GPIOs marked with ✅ are free to use. GPIO10 Can be used for analog input, but not for others.
- SD Pin If you don't use an SD card, these GPIOs(16,15,11,42) are free to use.
- ESP cannot be woken up by touch interrupt because RTC_GPIO is not connected to touch interrupt in design. However, GPIO10 can be connected to GPIO47 to make touch wake-up function possible. Please see here [issues/93](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/issues/93#issuecomment-2488500117)


### ⚡ Electrical parameters

| Features                | Details   |
| ----------------------- | --------- |
| 🔗USB-C Input Voltage    | 5V        |
| 🔋Battery Voltage        | 3.7V~4.2V |
| 📍**VBUS** ( Pin header) | 5V        |
| 📍**VBAT** ( Pin header) | 4.2V      |

* Molex connector and 3.3V Pin Header no more than 100mA load
* 5V power source from USB-C, load capacity depends on USB-C adapter
