// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-

#ifndef FLASCHEN_TASCHEN_H_
#define FLASCHEN_TASCHEN_H_

#include <stdint.h>

// hardcoded right now.
#define FLASCHEN_TASCHEN_HEIGHT 5

struct Color {
    Color() {}
    Color(int rr, int gg, int bb) : r(rr), g(gg), b(bb){}

    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Interfacing with the strip display. Sends content according to
// protocol described in /firmware/README.md
class FlaschenTaschen {
public:
    FlaschenTaschen(int width);
    ~FlaschenTaschen();

    int width() const { return size_ / FLASCHEN_TASCHEN_HEIGHT; }
    int height() const { return FLASCHEN_TASCHEN_HEIGHT; }

    void SetPixel(int x, int y, const Color &col);
    void Send();

private:
    const int size_;
    Color *strip_;
};

#endif // FLASCHEN_TASCHEN_H_
