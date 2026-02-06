#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

// User libraries
#include "./tun/tun_device.h"
#include "./network/network.h"
#include "./network/tcp/tcp.h"

// Return a pipe to a packet logging script
static int log_init() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fprintf(stderr, "[ERROR]: Could not initialize pipe ~ %s\n", strerror(errno));
        return -1;
    }

    int readfd = pipefd[0], writefd = pipefd[1];

    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "[ERROR]: Could not fork piping process ~ %s\n", strerror(errno));
        close(readfd);
        close(writefd);
        return -1;
    }

    // Child process
    if (pid == 0) {
        close(writefd);

        // Redirect py stdin to read pipe
        dup2(readfd, STDIN_FILENO);
        close(readfd);

        execl("/usr/bin/python3", "python3", "./log.py", NULL);

        close(readfd);
        close(writefd);
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(readfd); 

    return writefd;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "[USAGE ERROR]: ipaddress, devicename, (verboseoption)\n");
        exit(EXIT_FAILURE);
    }

    // Enforce string bounds
    if (strlen(argv[1]) >= INET_ADDRSTRLEN || strlen(argv[2]) >= IFNAMSIZ) {
        fprintf(stderr, "[USAGE ERROR]: ipaddress and devicename must be < 16 bytes\n");
        exit(EXIT_FAILURE);
    }
    
    // Copy arguments into memory
    char ip[INET_ADDRSTRLEN] = {'\0'};
    strncpy(ip, argv[1], INET_ADDRSTRLEN - 1);

    char tun_name[IFNAMSIZ] = {'\0'};
    strncpy(tun_name, argv[2], IFNAMSIZ - 1);

    bool verbose = (argc > 3 && strcmp(argv[3], "vb") == 0);

    // Initialize the device
    Tun *tun = tun_init(tun_name, ip);
    if (tun == NULL) 
        exit(EXIT_FAILURE);
    
    // Initialize log pipe on user request
    int log_fd;
    if (verbose) {
        printf(
            "TUN device:\n"
            "    Device name    : %s\n"
            "    IP             : %s\n"
            "    File descriptor: %d\n",
            tun->name,
            tun->ip,
            tun->fd
        );

        log_fd = log_init();
        if (log_fd < 0) {
            fprintf(stderr, "[ERROR]: Failed to initialize log pipe\n");
            tun_destroy(tun);
            exit(EXIT_FAILURE);
        }

        // Add SIGPIPE signal handler
        // When we write check 
    }

    uint8_t buf[MTU_SIZE + 1] = {'\0'};
    int nread;

    // Create an event structure and pass it to the tun function


    while (1) {
        nread = tun_read(tun, buf, MTU_SIZE);
        if (nread > 0) {
            int nwrite = net_demux(tun->fd, buf, nread);
            if (nwrite > 0) {
                tun_write(tun, buf, nwrite);
            }
        }
    }

    tun_destroy(tun);
}