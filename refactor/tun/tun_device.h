#ifndef TUN_DEVICE_H
#define TUN_DEVICE_H

// ═══════════════════════════════════════════════════════════════════════════
// TUN Device Interface
// ═══════════════════════════════════════════════════════════════════════════
//
// Provides a clean API for creating and managing Linux TUN devices without
// shelling out to system commands. All configuration is done via Netlink
// sockets directly to the kernel.
//
// Usage:
//   Tun *tun = tun_init("tun0", "10.8.0.1", true);
//   // Use tun->fd for read/write
//   tun_destroy(tun);
//

typedef struct Tun {
    char *name; // Device name (e.g., "tun0")
    char *ip;   // IP address string (e.g., "10.8.0.1")
    int fd;     // File descriptor for reading/writing packets
} Tun;

# define TUN_FILE_ERROR   -3  
# define TUN_KWRITE_ERROR -1  

# define TUN_DSOCK_ERROR  -2  
# define TUN_INET_ERROR   -4  
# define TUN_SEND_ERROR   -5  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <net/if.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

int tun_write(Tun *tun, uint8_t *buf, int len);
int tun_read(Tun *tun, uint8_t *buf, int len);
void tun_print(Tun *tun);

/**
 * Bring an interface up using Netlink.
 * Equivalent to: ip link set <name> up
 * 
 * @param name Device name (e.g., "tun0")
 * @return 0 on success, error code on failure.
 */
int tun_ip_wake(char *name);

/**
 * Add an IP address to an existing interface using Netlink.
 * Equivalent to: ip addr add <ip>/24 dev <name>
 * 
 * @param name Device name (e.g., "tun0")
 * @param ip   IP address in dotted-decimal notation (e.g., "10.8.0.1")
 * @return 0 on success, error code on failure.
 */
int tun_ip_add(char *name, char *ip);

/**
 * Allocate a TUN device via /dev/net/tun.
 * @param name Device name (max 16 bytes). If empty, kernel assigns a name.
 *             The actual assigned name is written back to this buffer.
 * @return File descriptor on success, error code on failure.
 */
int tun_alloc(char *name);

/**
 * Create and configure a TUN device in one call.
 * Allocates the device, adds the IP address, and brings it up.
 * 
 * @param name    Device name (max 16 bytes)
 * @param ip      IP address for the device
 * @return Tun structure on success, NULL on failure (check stderr).
 */
Tun *tun_init(char *name, char *ip);

/**
 * Tear down a TUN device and free all resources.
 * Does not delete the interface from the kernel — just closes our handle.
 * 
 * @param tun The device structure to destroy
 */
void tun_destroy(Tun *tun);

#endif