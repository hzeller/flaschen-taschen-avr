// -*- mode: cc; c-basic-offset: 4; indent-tabs-mode: nil; -*-
// Read http://openpixelcontrol.org/ and spit out our format.
// Currently: outputs stuff to tty, to be called with socat.

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "flaschen-taschen.h"

// todo: configurable.
#define FLASCHEN_TASCHEN_WIDTH 10

struct Header {
    uint8_t y_pos;
    uint8_t command;
    uint8_t size_hi;
    uint8_t size_lo;
};

static bool reliable_read(int fd, void *buf, size_t count) {
    ssize_t r;
    while ((r = read(fd, buf, count)) > 0) {
        count -= r;
        buf = (char*)buf + r;
    }
    return count == 0;
}

// Open server. Return file-descriptor or -1 if listen fails.
// Bind to "bind_addr" (can be NULL, then it is 0.0.0.0) and "port".
static int open_server(const char *bind_addr, int port) {
    if (port > 65535) {
        fprintf(stderr, "Invalid port %d\n", port);
        return -1;
    }
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        fprintf(stderr, "creating socket: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind_addr && !inet_pton(AF_INET, bind_addr, &serv_addr.sin_addr.s_addr)) {
        fprintf(stderr, "Invalid bind IP address %s\n", bind_addr);
        return -1;
    }
    serv_addr.sin_port = htons(port);
    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Trouble binding to %s:%d: %s",
                bind_addr ? bind_addr : "0.0.0.0", port,
                strerror(errno));
        return -1;
    }
    return s;
}

static void handle_connection(int fd) {
    FlaschenTaschen display(FLASCHEN_TASCHEN_WIDTH);
    bool any_error = false;
    while (!any_error) {
        struct Header h;
        if (!reliable_read(fd, &h, sizeof(h)))
            break;  // lazily assume that we get enough.
        uint16_t size = (uint16_t) h.size_hi << 16 | h.size_lo;
        int leds = size / 3;
        Color c;
        for (int x = 0; x < leds; ++x) {
            if (!reliable_read(fd, &c, 3)) {
                any_error = true;
                break;
            }
            display.SetPixel(x, h.y_pos, c);
        }
        if (size % 3 != 0) {
            fprintf(stderr, "Discarding wrongly sized data\n");
            reliable_read(fd, &c, size % 3);
        }
        display.Send();
    }
    fprintf(stderr, "Done.");
}

static void run_server(int listen_socket) {
    if (listen(listen_socket, 2) < 0) {
        fprintf(stderr, "listen() failed: %s", strerror(errno));
        return;
    }
    
    for (;;) {
        struct sockaddr_in client;
        socklen_t socklen = sizeof(client);
        int fd = accept(listen_socket, (struct sockaddr*) &client, &socklen);
        if (fd < 0) return;
        handle_connection(fd);
    }

    close(listen_socket);
}

int main() {
    int port = 7890;  // default port.
    int server_socket = open_server(NULL, port);
    fprintf(stderr, "Listening on %d\n", port);
    run_server(server_socket);
}
