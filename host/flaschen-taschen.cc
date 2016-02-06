// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-

#include "flaschen-taschen.h"

#include <assert.h>
#include <unistd.h>

FlaschenTaschen::FlaschenTaschen(int width)
    : size_(width * FLASCHEN_TASCHEN_HEIGHT),
      strip_(new Color[size_]) {
}

FlaschenTaschen::~FlaschenTaschen() { delete [] strip_; }

void FlaschenTaschen::SetPixel(int x, int y, const Color &col) {
    // Zig-zag assignment of our strips, so every other column has the
    // y-offset reverse.
    int y_off = y % FLASCHEN_TASCHEN_HEIGHT;
    int pos = (x * FLASCHEN_TASCHEN_HEIGHT) +
        ((x % 2 == 0) ? y_off : (FLASCHEN_TASCHEN_HEIGHT-1) - y_off);
    assert(pos < size_);
    strip_[pos] = col;
}

void FlaschenTaschen::Send() {
    uint8_t flow_info = 0xff;
    for (int i = 0; i < size_; ++i) {
        write(STDOUT_FILENO, &flow_info, 1);
        write(STDOUT_FILENO, &strip_[i].r, 1);
        write(STDOUT_FILENO, &strip_[i].g, 1);
        write(STDOUT_FILENO, &strip_[i].b, 1);
    }
    
    flow_info = 0;
    write(STDOUT_FILENO, &flow_info, 1);  // end of strip.
}
