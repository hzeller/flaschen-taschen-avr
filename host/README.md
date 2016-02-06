Host software
=============

Currently just very simple: push a number of pictures over
and let them scroll through. Size of display etc. are compile
time constants.

```
./send-image foo.ppm bar.ppm baz.ppm | socat STDIO /dev/ttyUSB1,raw,echo=0,crtscts=0,b115200
```

Run an http://openpixelcontrol.org/ server on standard port 7890:

```
./opc-server | socat STDIO /dev/ttyUSB1,raw,echo=0,crtscts=0,b115200
```
