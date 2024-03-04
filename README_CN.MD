<h1 align = "center">🌟LilyGo E-Paper 🌟</h1>

## **[English](./README.MD) | 中文**
- **T5-4.7-S3** 版本在[这里](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/tree/esp32s3)
- 驱动与示例程序来源于[vroland/epdiy](https://github.com/vroland/epdiy)

## 文档

您可以通过 [T5-4.7 & T5-4.7 Plus 在线文档](https://t5-47-t5-47-plus-zh-cn.readthedocs.io/zh_CN/latest/) 获取本项目的所有信息。

## 快速开始

- 在github中点击"Download ZIP",下载完成后,在Arduino 菜单栏中选择 项目 -> 加载库 -> 添加.ZIP库 
关于更多详细说明请参考 https://www.arduino.cc/en/Guide/Libraries
- 板子选择`ESP32 Dev Module`
- 工具 -> PSRAM -> Enable
- 在Arduino IDE (文件->示例程序->LilyGoEPD47->demo) 选择示例
- 选择正确的端口，然后点击上传
- 图片与文字生成请看[脚本使用说明](./scripts/README.MD)

## 必备

需要将 [arduino-esp32](https://github.com/espressif/arduino-esp32) 升级到 `2.0.3` 版本，才支持 `ESP32-S3`。

升级现有的开发环境，可以参考以下方法：

```shell
$ pwd
/c/Users/lyy/Documents/Arduino
$ cd hardware/espressif/esp32/
$ git checkout 2.0.3
$ git submodule update
```

## 相关项目

- [OpenWeather Display](https://github.com/Xinyuan-LilyGO/LilyGo-EPD-4-7-OWM-Weather-Display.git)
- [EPD47-HC08](https://github.com/Xinyuan-LilyGO/EPD47-HC08.git)
- [Variometer](https://github.com/Oganisyan/Variometer-LilyGoEPD47)

## 常问问题

1. 还剩余哪些GPIO可以自由使用?

    IO12。
    如果不使用触摸屏，还有IO12,、IO13、IO14和IO15可以自由使用。
    IO0 已被使用，无法编程。

2. 如何关闭蓝色 LED 灯？

    要关闭蓝色 LED，您必须调用 `epd_poweroff_all()`，这将关闭由 PWM_EN 控制的整个电源。 请注意，USB 供电时无法关闭。 它只能在电池供电时关闭。 详情请参考[examples/demo](./examples/demo)

3. 如何修复残影？

    请上传 [examples/screen_repair](./examples/screen_repair)，尝试消除残影。
    它不是100%能消除残影。
