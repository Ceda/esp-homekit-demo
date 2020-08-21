PROGRAM = led_strip

EXTRA_COMPONENTS = \
	extras/http-parser \
	$(abspath ../../components/esp-8266/cJSON) \
	$(abspath ../../components/common/wolfssl) \
	$(abspath ../../components/common/homekit)

FLASH_SIZE ?= 32
# FLASH_SIZE ?= 8
# HOMEKIT_SPI_FLASH_BASE_ADDR ?= 0x7A000

HOMEKIT_SPI_FLASH_BASE_ADDR=0x7A000

EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS

include $(SDK_PATH)/common.mk

include wifi.h

ESPPORT="/dev/tty.wchusbserial1d120"
LIBS += m

PIXEL_RGB=2
# CONFIG_TARGET_PLATFORM_ESP8266=true

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
