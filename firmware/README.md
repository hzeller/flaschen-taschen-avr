Firmware
========

Simply pushing things from the serial line to SPI or ws2812.

Look in the Makefile to choose `FT_DEVICE` ('flaschen taschen device'). Can be set there
or via environment variable.

```
 # First time, set the fuses.
 ATMEGA_ID=8    make fuse8    # ... if this is an atmega8
 ATMEGA_ID=328p make fuse328  # ... if this is a atmega328
 FT_DEVICE=ws2812 BPS_RATE=38400 ATMEGA_ID=328p make flash
```

If avrdude is not on /dev/ttyUSB0, set in environment variable `AVRDUDE_DEVICE=/dev/ttyUSB0`. There are other environment variables of interest in the Makefile.
