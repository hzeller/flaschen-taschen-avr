# <h.zeller@acm.org>
##

# CPU frequency and baudrate as used at compile time. We want to have
# a slow communication as we have pretty long lines and don't use RS422, so
# consider to reduce the baudrate for production use to something that works on
# the line.

BPS_RATE?=115200

# Atmega ID can be '8' for Atmega8; 48, 88, 168, 328, 328p for Atmega48 etc.
ATMEGA_ID?=8

# FlaschenTaschen device. Can be lpd6803 or ws2811
FT_DEVICE?=lpd6803
#FT_DEVICE=ws2812

# Paramters for avrdude
AVRPROGRAMMER ?= stk500v2
AVRDUDE_DEVICE ?= /dev/ttyUSB0

GIT_VERSION="$(shell git log --date=short --pretty=format:'[%h] @ %cd' -n 1)"
DEFINES=-DGIT_VERSION='$(GIT_VERSION)' -DF_CPU=8000000UL -DSERIAL_BAUDRATE=$(BPS_RATE)

ifeq (x$(AVRDUDE_DEVICE),x)
	AVR_PATH=
else
	AVR_PATH=-P $(AVRDUDE_DEVICE)
endif
AVRDUDE     = avrdude -p m$(ATMEGA_ID) -c $(AVRPROGRAMMER) $(AVR_PATH)
TARGET_ARCH=-mmcu=atmega$(ATMEGA_ID)

WS2811_BASEDIR=light_ws2812/light_ws2812_AVR/Light_WS2812
CC=avr-gcc
CXX=avr-g++
CXXFLAGS=-Os -g -Wall \
    -ffunction-sections -fdata-sections -mcall-prologues \
    -fpack-struct -fno-move-loop-invariants -fno-tree-scev-cprop \
    -fno-inline-small-functions \
    $(DEFINES) $(TARGET_ARCH) -I. -I$(WS2811_BASEDIR)
FLASH_CMD   = $(AVRDUDE) -e -U flash:w:main.hex
LINK=avr-g++ -g $(TARGET_ARCH) -Wl,-gc-sections
OBJECTS=$(FT_DEVICE)-main.o serial-com.o light_ws2812.o

all : main.hex

main.elf: $(OBJECTS)
	$(LINK) -o $@ $(OBJECTS)
	avr-size $@

disasm: main.elf
	avr-objdump -C -S main.elf

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

flash: main.hex
	$(FLASH_CMD)

%.o : %.cc compiler-flags
	$(CXX) $(CXXFLAGS) -c -o $@ $<

light_ws2812.o: $(WS2811_BASEDIR)/light_ws2812.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

# Make sure to re-compile when the compiler flags change.
compiler-flags: FORCE
	@echo '$(CXXFLAGS)' | cmp -s - $@ || echo '$(CXXFLAGS)' > $@

.PHONY: FORCE

clean:
	rm -f $(OBJECTS) main.elf main.hex

# Documentation page references from
# http://www.atmel.com/images/atmel-2486-8-bit-avr-microcontroller-atmega8_l_datasheet.pdf / pp 216-217 are interesting.
# internal oscillator.
### Fuse high byte:
# 7 rstdisbl	1   disable external reset: disabled (i.e.: reset enabled).
# 6 wdton	1   watchdog.
# - spien	0   serial programming: enabled.
# 4 chkopt	1   clock options (doesn't matter, we do internal RC)
#
# 3 eesave      0   save eeprom on chip erase: enabled
# 2 bootsz1	1
# 1 bootsz0	1
# 0 bootrst     1   starting at 0x0000

### low byte:
# 7 bodlevel	1   brown out detect at 2.7v
# 6 boden	0   brown out enable.
# 5 sut1	1-+ slow rising power (page 30)
# 4 sut0	0/
#
# 3 cksel3	0\
# 2 cksel2	1 + internal RC oscillator  (page 30)  (8Mhz)
# 1 cksel1	0/
# 0 cksel0	0
fuse8:
	$(AVRDUDE) -U hfuse:w:0xd7:m -U lfuse:w:0xe4:m

# similar for 328. Fuses on page 298
# Fuse high byte:
### Fuse high byte:
# 7 rstdisbl	1   disable external reset: disabled (i.e.: reset enabled).
# 6 dwen	1   debug wire.
# - spien	0   serial programming: enabled.
# 4 wdton	1   watchdog.
#
# 3 eesave      0   save eeprom on chip erase: enabled
# 2 bootsz1	0
# 1 bootsz0	0
# 0 bootrst     0  # start at 000
fuse328:
	$(AVRDUDE) -U hfuse:w:0xd0:m -U lfuse:w:0xe2:m -U efuse:w:0x05:m
