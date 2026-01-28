#ifndef V_DEVICE_H
#define V_DEVICE_H

#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct Virtual_Device Virtual_Device;

struct Virtual_Device {
    char *dev;
    char *ip;
    int flags;
    int fd;
};

# define MTU_SIZE 1500

/**
 * @brief Creates a tun device
 * @param dev the name of an interface, or lets the kernel choose with '\0' 
 * @param flags the interface flags
 * @return the file descriptor of the binded interface, or -1 on error
 */
int tun_alloc(char *dev, int flags);

/** 
 * @brief Bind a virtual interface to an ip address
 * @param vd a wrapper including ip, flags, device name, fd
 * @return -1 on error, or 0 on successful mapping to the IP address
*/
int tun_init(bool verbose, Virtual_Device *vd);

/**
 * @brief Cleanup the contents of a stack allocated virtual device
 * @param vd the the device to destroy
 */
void virtual_device_destroy(Virtual_Device vd);

#endif


