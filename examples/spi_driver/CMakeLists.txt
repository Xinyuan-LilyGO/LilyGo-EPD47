cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "../../src")
list(APPEND EXTRA_COMPONENT_DIRS "../../src/libjpeg")
list(APPEND EXTRA_COMPONENT_DIRS "../../src/zlib")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(spi_driver)
