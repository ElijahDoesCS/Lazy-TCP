#include "./log.h"

// Get nanosecond timestamp
uint64_t log_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

int log_init() {
    FILE *fifo_end = fopen("./env/fifo_end.txt", "r");
    if (fifo_end == NULL)
        return -1;

    char fifo_path[64] = {0};
    fread(fifo_path, 63, sizeof(char), fifo_end);  

    unlink(fifo_path);
    if (mkfifo(fifo_path, 0600) < 0) {
        printf("make fifo fuckin failed bro\n");
        return -1;
    }

    printf("[LOG]: Waiting for logger to attach to FIFO...\n");

    int fd = open(fifo_path, O_WRONLY);
    if (fd < 0) {
        printf("we made the fifo but we couldn't fucking open it?\n");
        printf("Here is the path: %s\n", fifo_path);
        return -1;
    }

    printf("[LOG]: Logger attached to FIFO\n");

    return fd;
}