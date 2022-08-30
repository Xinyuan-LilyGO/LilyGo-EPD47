# EPD SPI Example

In this example, esp32 is used as the driver chip of epd (instead of it8951), and other devices can control epd through spi.

## How to use example

### Hardware Requirement

* LILYGO 4.7" EPD
* Raspberry Pi 3 Model B

### Hardware Connection

| BCM Pin | Raspberry-Pi Pin | Function | Description |
| ------- | ---------------- | -------- | ------------ |
| 11      | 23               | SCLK     | SPI Clock pin |
| 10      | 19               | MOSI     | SPI MOSI pin |
| 9       | 21               | MISO     | SPI MOSI pin |
| 8       | 24               | CS       | Chip Select input pin(SPI) |

### Build and Flashing

#### Arduino

Refer to the use of esp32 Arduino

### Test

Execute the following commands on the Raspberry Pi:

```shell
$ pwd
LilyGo-EPD47/examples/spi_driver/test
$ pip3 install spidev
$ python3 epd.py
jpeg size= 5586
send jpeg time:  0.12708497047424316 s
jpeg size= 45342
send jpeg time:  0.42786264419555664 s
img size= 20000
send img time:  0.18109393119812012 s
img size= 259200
send img time:  1.971804141998291 s
```

## Troubleshooting

If the screen cannot display the image normally, please check whether the connection port or the screen connection cable is loose.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, please contact the customer service of the purchase site
* For a feature request or bug report, create a [GitHub issue](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/issues)

We will get back to you as soon as possible.
