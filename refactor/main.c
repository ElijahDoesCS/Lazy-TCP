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

static volatile sig_atomic_t g_shutdown = 0;

static void server_shutdown(int sig) {
    (void)sig;
    g_shutdown = 1;
}

void server_run(uint8_t *tu, int mtu, Tun *tun, int log_fd) {
    int nread, log = (log_fd > 0);

    while (!g_shutdown) {
        nread = tun_read(tun, tu, mtu);
        if (nread > 0) {
            if (log) {
                Event event = {
                    .dir = 1,
                    .timestamp_ns =  
                };
                
            }

            int nwrite = net_demux(tun->fd, tu, nread);
            if (nwrite > 0) {
                tun_write(tun, tu, nwrite);

                if (log) {

                }
            }
        }

    }

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
    
    // Initialize debug
    int log_fd = -1;
    if (verbose) 
        log_fd = log_init();
    
    if (verbose && log_fd == -1) {
        fprintf(stderr, "[ERROR]: Could not initialize log device ~ %s\n", strerror(errno));
        tun_destroy(tun);
        exit(EXIT_FAILURE);
    }

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
    }

    // Initialize resource deallocation handler
    struct sigaction sa_usr = {0};
    sa_usr.sa_handler = server_shutdown;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa_usr, NULL);

    uint8_t buf[MTU_SIZE + 1] = {'\0'};
    server_run(buf, MTU_SIZE, tun, log_fd);    

    if (log_fd > 0)
        close(log_fd);    
    // tcb_free
    tun_destroy(tun);
}