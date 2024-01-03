#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"

ssize_t read_all(int fd, void* arg, size_t count) {
    size_t bytes_left = count;

    do {
        ssize_t out = read(fd, arg, bytes_left);
        if(out == -1) {
            fprintf(stderr, "Read failed\n");
            return 1;
        }
        size_t bytes_read = (size_t)out;

        arg = arg + (sizeof(size_t) * bytes_read);
        bytes_left -= bytes_read;
        if (bytes_left > 0) {
            fprintf(stderr, "Didn't read all at once (%zu/ %zu)\n", count-bytes_left, count);
        }
    } while (bytes_left != 0);

    return 0;
}

void write_all(int fd, void* arg, size_t count) {
    size_t bytes_left = count;

    do {
        ssize_t out = write(fd, arg, bytes_left);
        if(out == -1) {
            fprintf(stderr, "Write failed\n");
        }
        size_t bytes_writen = (size_t)out;

        arg = arg + (sizeof(size_t) * (size_t)bytes_writen);
        bytes_left -= bytes_writen;
        if (bytes_left > 0) {
            fprintf(stderr, "Didn't write all at once (%zu/ %zu)\n", count-bytes_left, count);
        }
    } while (bytes_left != 0);
}