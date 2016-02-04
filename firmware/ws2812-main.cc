/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * Copyright (c) h.zeller@acm.org. GNU public License.
 */
#include "serial-com.h"
extern "C" {
#include "light_ws2812.h"
}

// Pixels we have available in the strip. Needs to be
// less than ram-size/3
#define FLASCHEN_TASCHEN_PIXELS 256

static void SendStrip(SerialCom *com) {
    static cRGB strip[FLASCHEN_TASCHEN_PIXELS];
    for (uint8_t i = 0; /**/; ++i) {
        if (com->read() == 0)  // first byte is !0 if there is data.
            break;
        if (i >= FLASCHEN_TASCHEN_PIXELS) i = 0; // Uh, got more than needed.
        strip[i].r = com->read();
        strip[i].g = com->read();
        strip[i].b = com->read();
    }

    ws2812_setleds(strip, FLASCHEN_TASCHEN_PIXELS);

    for (const char *out = "ok"; *out; ++out)
        com->write(*out);
}

int main() {
    SerialCom comm;

    const char *hello = GIT_VERSION;
    while (*hello) comm.write(*hello++);

    for (;;) {
        SendStrip(&comm);
    }
}
