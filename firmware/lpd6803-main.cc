/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * Copyright (c) h.zeller@acm.org. GNU public License.
 */
#include <avr/io.h>

#include "serial-com.h"

// Current pixels.
#define FLASCHEN_TASCHEN_PIXELS 50

#define SPI_MODE0 0x00
#define SPI_MODE_MASK 0x0C

enum SpiBitOrder {
    LSBFIRST,
    MSBFIRST,
};

// The bits on port B that we're using.
// Hardcoded, as we're using a specific
// chip. Atmega8
enum PortBBits {
    PIN_SS = 2,
    PIN_MISO = 4,
    PIN_MOSI = 3,
    PIN_SCK = 5,
    PIN_RST = 1,
};

class SPI {
public:
    static void Init(uint8_t chip_select) {
        PORTB |= (1<<chip_select);  // set chip select high initially.
        DDRB |= (1<<chip_select) | (1<<PIN_SCK) | (1<<PIN_MOSI);
        SPCR |= _BV(MSTR);
        SPCR |= _BV(SPE);
    }

    static void setBitOrder(SpiBitOrder bitOrder) {
        if(bitOrder == LSBFIRST) {
            SPCR |= _BV(DORD);
        } else {
            SPCR &= ~(_BV(DORD));
        }
    }

    static void setDataMode(unsigned char mode) {
        SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
    }

    static unsigned char transfer(unsigned char data) {
        SPDR = data;
        while (!(SPSR & _BV(SPIF)))
            ;
        return SPDR;
    }
};

// Directly send data from the strip as it comes from the serial
// line. The end of one record is marked with a nul-byte.
static void SendStrip(SerialCom *com) {
    // LPD6803 start sequence
    for (int i = 0; i < 4; ++i)
        SPI::transfer(0);

    for (int i = 0; i < FLASCHEN_TASCHEN_PIXELS; ++i) {
        uint8_t upper = com->read();
        // Shorter lengths can be sent if followed by nul-byte.
        if (upper == 0) break;        // Upper bit should've been set.
        uint8_t lower = com->read();
        SPI::transfer(upper);
        SPI::transfer(lower);
    }
    for (const char *out = "ok"; *out; ++out)
        com->write(*out);
}

int main() {
    SPI::Init(PIN_SS);
    SPI::setBitOrder(MSBFIRST);

    SerialCom comm;

    const char *hello = GIT_VERSION;
    while (*hello) comm.write(*hello++);

    for (;;) {
        SendStrip(&comm);
    }
}
