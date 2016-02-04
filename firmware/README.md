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

Current protocol: Command byte plus data needed for that command.

To set the colors in a pixel strip, the command byte is 0xff (really anything
other than 0x00): Each pixel-block starts with a command byte
followed by the color information; repeat until all pixels are set.

The data transmission is finished with a single 0x00-byte.

| 0xff | r | g | b | 0xff | r | g | b | 0xff | r | g | b | 0x00 |
|------|---|---|---|------|---|---|---|------|---|---|---|------|

Firmware responds with ASCII `ok` after it successfully received a command
ending with `0x00`.

(If it turns out that sending strips is all we ever do, this can be simplifid:
single byte (or two?) indicating the length of the strip to follow,
then plain RGB sequence 3 bytes each and no end-byte. Saves 25% bandwidth, but
harder to recover from transmission errors).
