MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST))) # get current makefile path
# EPD47_PATH := $(patsubst %/, %, $(dir $(MAKEFILE_PATH))) # get current dir path
EPD47_PATH := $(shell dirname $(MAKEFILE_PATH))
$(info EPD47_PATH=$(EPD47_PATH))
EXTRA_COMPONENT_DIRS += $(EPD47_PATH)/src/
EXTRA_COMPONENT_DIRS += $(EPD47_PATH)/src/zlib
EXTRA_COMPONENT_DIRS += $(EPD47_PATH)/src/libjpeg
