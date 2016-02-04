// -*- mode: cc; c-basic-offset: 4; indent-tabs-mode: nil; -*-
// quick hack.
// strip data output on stdout, so use in a pipe with socat
// ./send-image foo.ppm bar.ppm | socat STDIO /dev/ttyUSB1,raw,echo=0,crtscts=0,b115200

// BSD-source for usleep()
#define _BSD_SOURCE

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>

#define FLASCHEN_TASCHEN_WIDTH	10
#define FLASCHEN_TASCHEN_HEIGHT 5
#define FLASCHEN_TASCHEN_PIXELS (FLASCHEN_TASCHEN_WIDTH*FLASCHEN_TASCHEN_HEIGHT)

struct Color {
    Color() {}
    Color(int rr, int gg, int bb) : r(rr), g(gg), b(bb){}

    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Abstract display
class Display {
public:
    Display(int width)
        : size_(width * FLASCHEN_TASCHEN_HEIGHT),
          strip_(new Color[size_]) {
    }

    ~Display() { delete [] strip_; }

    int width() const { return size_ / FLASCHEN_TASCHEN_HEIGHT; }
    int height() const { return FLASCHEN_TASCHEN_HEIGHT; }

    void SetPixel(int x, int y, const Color &col) {
        //fprintf(stderr, "%d x %d\n", x, y);
        // Zig-zag assignment of our strips, so every other column has the
        // y-offset reverse.
        int y_off = y % FLASCHEN_TASCHEN_HEIGHT;
        int pos = (x * FLASCHEN_TASCHEN_HEIGHT) +
            ((x % 2 == 0) ? y_off : (FLASCHEN_TASCHEN_HEIGHT-1) - y_off);
        assert(pos < size_);
        strip_[pos] = col;
    }

    void send() {
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

private:
    const int size_;
    Color *strip_;
};


// Read line, skip comments.
char *ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
        result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
}

// Load PPM and return end of strip.
std::vector<Color> *LoadPPM(FILE *f) {
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: |%s", m, line); \
        fclose(f); return NULL; }

    if (f == NULL)
        return NULL;
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
    if (height != FLASCHEN_TASCHEN_HEIGHT || width < FLASCHEN_TASCHEN_WIDTH) {
        EXIT_WITH_MSG("Uh, FlaschenTaschen is only height 5 right now; at least 10 wide.");
    }

    std::vector<Color> *result = new std::vector<Color>();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color c;
            fread(&c, 3, 1, f);
            result->push_back(c);
        }
    }
#undef EXIT_WITH_MSG
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {  // Limit to one picture right now.
        fprintf(stderr, "Expected picture.\n");
        return 1;
    }

    const int images = argc - 1;
    std::vector<Color> *image;
    // Read images, store strip compatible data.
    for (int i = 0; i < images; ++i) {
        FILE *f = fopen(argv[i + 1], "r");
        if (!(image = LoadPPM(f))) {
            fprintf(stderr, "Too bad, couldn't read everything\n");
            return 1;
        }
        fclose(f);
    }
    const int image_width = image->size() / FLASCHEN_TASCHEN_HEIGHT;
    fprintf(stderr, "Got image (width=%d)\n", image_width);

    const int direction = 1;
    const int sleep_ms = 200;
    int scroll_start = 0;

    Display display(FLASCHEN_TASCHEN_WIDTH);

    for (;;) {
        // Copy a part of our larger image, starting at scroll_start x-pos
        // into our display sized window.
        for (int x = 0; x < display.width(); ++x) {
            for (int y = 0; y < display.height(); ++y) {
                // The x-position in our image changes while we scroll.
                const int img_x = (scroll_start + image_width + x) % image_width;
                const Color &pixel_color = (*image)[img_x + y * image_width];
                // Our display is upside down. Lets mirror.
                display.SetPixel(FLASCHEN_TASCHEN_WIDTH - x - 1,
                                 FLASCHEN_TASCHEN_HEIGHT - y - 1,
                                 pixel_color);
            }
        }

        display.send();  // ... and show it.

        if (image_width == display.width()) {
            // Image fits. No scrolling needed.
            return 0;
        }
        usleep(sleep_ms * 1000);
        scroll_start = (scroll_start + direction) % image_width;
    }

    return 0;
}
