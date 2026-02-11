#define _POSIX_C_SOURCE 200809L

#include "./log.h"

int g_log_fd = -1;

uint64_t log_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void log_event(uint8_t *tu, uint16_t len, uint8_t dir, int fd) {
    Event event = {
        .dir = dir,
        .timestamp_ns = log_time(),
        .tu_len = len,
        .tu = {0}
        // .state = {0}
    };

    if (tu) memcpy(&(event.tu), tu, len);

    ssize_t result = write(fd, &event, sizeof(Event));
    if (result == -1) {
        if (errno == EPIPE) {
            printf("[LOG]: Debugging instance exited\n");
        }
        else {
            fprintf(stderr, "[ERROR]: Write to debug failed ~ %s\n", strerror(errno));
        }

        g_log_fd = -1;
    }
}

int log_init() {
    FILE *fifo_end = fopen("./env/fifo_end.txt", "r");
    if (fifo_end == NULL)
        return -1;

    char fifo_path[64] = {0};
    fread(fifo_path, 63, sizeof(char), fifo_end);  

    unlink(fifo_path);
    if (mkfifo(fifo_path, 0600) < 0) 
        return -1;

    printf("[LOG]: Waiting for logger to attach to FIFO...\n");

    int fd = open(fifo_path, O_WRONLY);
    if (fd < 0) 
        return -1;
    
    printf("[LOG]: Logger attached to FIFO\n");

    return fd;
}