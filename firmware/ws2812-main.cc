/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * Copyright (c) h.zeller@acm.org. GNU public License.
 */
#include <avr/io.h>

#include "serial-com.h"
extern "C" {
#include "light_ws2812.h"
}

// Current pixels.
#define FLASCHEN_TASCHEN_PIXELS 50

static cRGB strip[FLASCHEN_TASCHEN_PIXELS];

// Directly send data from the strip as it comes from the serial
// line. The end of one record is marked with a nul-byte.
static void SendStrip(SerialCom *com) {
    for (uint8_t i = 0; /**/; ++i) {
        if (com->read() == 0)  // first byte is !0 if there is data.
            break;
        if (i >= FLASCHEN_TASCHEN_PIXELS) i = 0; // Uh, got more than needed.
        strip[i].r = com->read();
        strip[i].g = com->read();
        strip[i].b = com->read();
    }
    for (const char *out = "ok"; *out; ++out)
        com->write(*out);
    ws2812_setleds(strip, FLASCHEN_TASCHEN_PIXELS);
}

int main() {
    SerialCom comm;

    const char *hello = GIT_VERSION;
    while (*hello) comm.write(*hello++);

    for (;;) {
        SendStrip(&comm);
    }
}
