// -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
// quick hack.
// strip data output on stdout, so use in a pipe with socat
// ./send-image foo.ppm bar.ppm | socat STDIO /dev/ttyUSB1,raw,echo=0,crtscts=0,b115200

// BSD-source for usleep()
#define _BSD_SOURCE

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FLASCHEN_TASCHEN_WIDTH	10
#define FLASCHEN_TASCHEN_HEIGHT 5
#define FLASCHEN_TASCHEN_PIXELS (FLASCHEN_TASCHEN_WIDTH*FLASCHEN_TASCHEN_HEIGHT)

void SetRGB(int x, int y, int r, int g, int b,
	    uint16_t *strip) {
    // Zig-zag assignment of our strips, so every other column has the
    // y-offset reverse.
    int y_off = y % FLASCHEN_TASCHEN_HEIGHT;
    int pos = (x * FLASCHEN_TASCHEN_HEIGHT) +
        ((x % 2 == 0) ? y_off : (FLASCHEN_TASCHEN_HEIGHT-1) - y_off);

    // Data for the LPD6803 https://www.adafruit.com/datasheets/LPD6803.pdf 
    uint16_t data = 0;
    data |= (1<<15);  // start bit
    data |= ((r/8) & 0x1F) << 10;
    data |= ((g/8) & 0x1F) <<  5;
    data |= ((b/8) & 0x1F) <<  0;
    strip[pos] = data;
}

// Read line, skip comments.
char *ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
        result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
}

int LoadPPM(FILE *f,  uint16_t *strip) {
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: |%s", m, line); \
        fclose(f); return 0; }

    if (f == NULL)
        return 0;
    char header_buf[256];
    // Header contains widht and height.
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
    if (sscanf(line, "P6 ") == EOF)
        EXIT_WITH_MSG("Can only handle P6 as PPM type.");
    line = ReadLine(f, header_buf, sizeof(header_buf));
    int width, height;
    if (!line || sscanf(line, "%d %d ", &width, &height) != 2)
        EXIT_WITH_MSG("Width/height expected");
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
        EXIT_WITH_MSG("Only 255 for maxval allowed.");
	
    // Hardcoded pixel mapping
    if (width != 10 && height != 5) {
        EXIT_WITH_MSG("Uh, FlaschenTaschen is only sized 10x5 now");
    }

    for (int y = 0; y < height; ++y) {	
        for (int x = 0; x < width; ++x) {
            uint8_t rgb[3];
            fread(rgb, 3, 1, f);
            SetRGB(width - 1 - x, height - 1 - y, rgb[0], rgb[1], rgb[2], strip);
        }
    }
#undef EXIT_WITH_MSG
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Expected picture(s).\n");
        return 1;
    }

    const int images = argc - 1;
    uint16_t *buffer;
    buffer = (uint16_t*) malloc(sizeof(uint16_t) * FLASCHEN_TASCHEN_PIXELS * images);

    // Read images, store strip compatible data.
    for (int i = 0; i < images; ++i) {
        FILE *f = fopen(argv[i + 1], "r");
        uint16_t *strip = buffer + i*FLASCHEN_TASCHEN_PIXELS;
        if (!LoadPPM(f, strip)) {
            fprintf(stderr, "Too bad, couldn't read everything\n");
            return 1;
        }
        fclose(f);
    }
    fprintf(stderr, "Got %d images\n", images);

    const int direction = -1;
    const int sleep_ms = 200;

    const int scroll_advance = direction * FLASCHEN_TASCHEN_HEIGHT;
    // If we have multiple images, scroll through these.
    const int total_pixels = FLASCHEN_TASCHEN_PIXELS * images;
    int scroll_start = 0;

    // Now scroll all these through
    for (;;) {
        // From our current scroll-start, show all the pictures.
        for (int i = 0; i < FLASCHEN_TASCHEN_PIXELS; ++i) {
            uint16_t data = buffer[(scroll_start + i + total_pixels) % total_pixels];
            uint8_t upper = data >> 8;
            uint8_t lower = data & 0xFF;
            write(STDOUT_FILENO, &upper, 1);
            write(STDOUT_FILENO, &lower, 1);
        }

        uint8_t dummy = 0;  // end sentinel accepted by the firmware.
        write(STDOUT_FILENO, &dummy, 1);
        
        if (images == 1)  // One image. We're done.
            return 0;
        usleep(sleep_ms * 1000);
        // Scroll.
        scroll_start = (scroll_start + scroll_advance + total_pixels)
            % total_pixels;
    }
}
