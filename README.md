[ Obsolete. Check https://github.com/hzeller/flaschen-taschen instead ]

Quick hack on 2016-01-30, to get some first light out of the [FlaschenTaschen]
project. We just had an RFID [terminal PCB][terminal] lying around, so this
was a quick re-purpose to talk to the LPD6803 or ws2812 strips and wiring them up to a serial
line.

Use --recursive for cloning to get the ws2812 sub-module

```
git clone --recursive https://github.com/hzeller/flaschen-taschen-avr.git
```

[FlaschenTaschen]: https://noisebridge.net/wiki/Flaschen_Taschen
[terminal]: https://github.com/hzeller/rfid-access-control/tree/master/hardware/terminal
