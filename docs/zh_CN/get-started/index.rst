********
快速入门
********

本文档旨在指导用户搭建 T5-4.7 和 T5-4.7 Plus 硬件开发的软件环境。

安装准备
==========

请先完成工具的安装，具体步骤见下：

+-------------------+-------------------+
| |arduino-logo|    | |platformio-logo| |
+-------------------+-------------------+
| `arduino`_        | `platformio`_     |
+-------------------+-------------------+

.. |arduino-logo| image:: ../../_static/arduino-logo.png
    :target: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide

.. |platformio-logo| image:: ../../_static/platformio-logo.png
    :target: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-platformio

.. _arduino: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide
.. _platformio: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-platformio

安装依赖库
==========

使用“下载 ZIP”按钮从 github 下载 zip 文件并使用 IDEA 安装它 (:code:`Sketch`` -> :code:`Include Library` -> :code:`Add .ZIP Library...`).

* `PCF8563_Library`_
* `LilyGoEPD47`_

.. _PCF8563_Library: https://github.com/lewisxhe/PCF8563_Library
.. _LilyGoEPD47: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47

Arduino编译配置
===================

- `T5-4.7 <https://www.aliexpress.us/item/3256801819744140.html>`_

    .. image:: ../../_static/t5-4.7-arduino-config.jpg

- `T5-4.7-Plus <https://www.aliexpress.us/item/3256804461011991.html>`_

    .. image:: ../../_static/t5-4.7-plus-arduino-config.jpg

原理图
=======

* `T5-4.7 Schematic`_ (pdf)
* `T5-4.7 Plus Schematic`_ (pdf)

.. _T5-4.7 Schematic: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/blob/master/schematic/T5-4.7.pdf
.. _T5-4.7 Plus Schematic: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/blob/master/schematic/T5-4.7-Plus.pdf

数据手册
========

* `ESP32`_ (Datasheet)
* `ESP32-WROVER-E`_ (Datasheet)
* `ESP32-S3`_ (Datasheet)
* `ESP32-S3-WROOM-1`_ (Datasheet)
* `ED047TC1`_ (Datasheet)
* `PCF8563`_ (Datasheet)


.. _ESP32: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_cn.pdf
.. _ESP32-WROVER-E: https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_cn.pdf
.. _ESP32-S3: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf
.. _ESP32-S3-WROOM-1: https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_cn.pdf
.. _ED047TC1: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/blob/master/datasheet/ED047TC1.pdf
.. _PCF8563: https://www.nxp.com.cn/docs/zh/data-sheet/PCF8563.pdf
