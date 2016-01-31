Firmware
========

Simply pushing things from the serial line to SPI

```
 make fuse  # first time
 make flash
```

If avrdude is not on /dev/ttyUSB0, set in environment variable `AVRDUDE_DEVICE=/dev/ttyUSB0`

