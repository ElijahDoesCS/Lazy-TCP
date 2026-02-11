#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// User libraries
#include "./tun/tun_device.h"
#include "./network/network.h"
#include "./network/tcp/tcp.h"
#include "./network/ip.h"
#include "./log/log.h"

static volatile sig_atomic_t g_shutdown = 0; // Shutdown the server and cleanup resources

static void server_shutdown(int sig) {
    (void)sig;
    g_shutdown = 1;

    if (VERBOSE(g_log_fd))
        printf("[LOG]: Requested server teardown...\n");
}

void server_run(uint8_t *tu, int mtu, Tun *tun) {
    while (!g_shutdown) {
        int nread = tun_read(tun, tu, mtu);
        int nwrite = 0;
        if (nread > 0) {
            LOG_EVENT(tu, nread, LOG_READ);
            nwrite = net_demux(tun->fd, tu, nread);
        }
        if (nwrite > 0) {
            LOG_EVENT(tu, nwrite, LOG_WRITE);
            tun_write(tun, tu, nwrite);
        } 
    }

    LOG_EVENT(NULL, 0, LOG_SHUTDOWN);
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

    // Initialize resource deallocation handler
    struct sigaction sa_usr = {0};
    sa_usr.sa_handler = server_shutdown;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa_usr, NULL);

    if (verbose) {
        signal(SIGPIPE, SIG_IGN);
        g_log_fd = log_init();

        if (!VERBOSE(g_log_fd)) {
            fprintf(stderr, "[ERROR]: Could not initialize log device ~ %s\n", strerror(errno));
            tun_destroy(tun);
            exit(EXIT_FAILURE);
        }

        printf(
            "TUN device:\n"
            "    Device name    : %s\n"
            "    IP             : %s\n"
            "    File descriptor: %d\n",
            tun->name,
            tun->ip,
            tun->fd
        );
    }

    uint8_t buf[MTU_SIZE] = {'\0'};
    server_run(buf, MTU_SIZE, tun);    

    if (VERBOSE(g_log_fd)) {
        printf("[LOG]: Freeing server resources\n");
        close(g_log_fd);
    }
        
    // tcb_free
    tun_destroy(tun);
}