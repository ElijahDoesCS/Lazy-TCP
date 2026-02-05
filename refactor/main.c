#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "./tun/tun_device.h"
#include "./network/network.h"

bool g_verbose = false;

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

    g_verbose = (argc > 3 && strcmp(argv[3], "vb") == 0);

    Tun *tun = tun_init(tun_name, ip);
    if (tun == NULL) exit(EXIT_FAILURE);
    
    uint8_t buf[MTU_SIZE + 1] = {'\0'};
    int nread;
    
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