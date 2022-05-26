# EPD SPI Example

该示例, 将esp32作为epd的驱动芯片(代替it8951), 其他设备可以通过spi对epd进行控制.

## 快速开始

### 硬件准备

* LILYGO 4.7" EPD
* Raspberry Pi 3 Model B

### 硬件连接

| BCM Pin | Raspberry-Pi Pin | Function | Description |
| ------- | ---------------- | -------- | ------------ |
| 11      | 23               | SCLK     | SPI Clock pin |
| 10      | 19               | MOSI     | SPI MOSI pin |
| 9       | 21               | MISO     | SPI MOSI pin |
| 8       | 24               | CS       | Chip Select input pin(SPI) |

### 编译及运行

#### Arduino

参看esp32 Arduino的编译

### 测试

在树莓派执行以下指令:

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

## 故障排除

如果屏幕无法正常显示图像, 请检查连接口或者屏幕连接排线是否有松动.

## 技术支持和反馈

请使用以下反馈渠道:

* 有关技术问题, 请联系购买网站的客服
* 对于功能请求或错误报告，创建一个 GitHub 问题

我们会尽快回复您.
