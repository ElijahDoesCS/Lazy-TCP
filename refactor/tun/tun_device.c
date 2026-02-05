#include "./tun_device.h"
#include "../debug/debug.h"

int tun_read(Tun *tun, uint8_t *buf, int len) {
    return read(tun->fd, buf, len);
}

int tun_write(Tun *tun, uint8_t *buf, int len) {
    return write(tun->fd, buf, len);
}

int tun_ip_wake(char *name) {
    // Open a Netlink socket to talk to the kernel's routing subsystem
    int sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (sock < 0) 
        return TUN_DSOCK_ERROR;

    // Build a Netlink message to modify link properties
    struct {
        struct nlmsghdr  nlh;
        struct ifinfomsg ifi;
    } req;

    memset(&req, 0, sizeof(req));

    // Netlink header: tell kernel we want to modify a link
    req.nlh.nlmsg_len   = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nlh.nlmsg_type  = RTM_NEWLINK;
    req.nlh.nlmsg_flags = NLM_F_REQUEST;
    req.nlh.nlmsg_seq   = 1;

    // Interface info: set the IFF_UP flag
    req.ifi.ifi_family = AF_UNSPEC;
    req.ifi.ifi_index  = if_nametoindex(name);
    req.ifi.ifi_flags  = IFF_UP;
    req.ifi.ifi_change = IFF_UP;

    // Send the message to the kernel
    if (send(sock, &req, req.nlh.nlmsg_len, 0) < 0) {
        close(sock);
        return TUN_SEND_ERROR;
    }

    close(sock);
    return 0;
}

int tun_ip_add(char *name, char *ip) {
    // Open a Netlink socket
    int sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (sock < 0) 
        return TUN_DSOCK_ERROR;

    // Build a Netlink message to add an address
    struct {
        struct nlmsghdr  nlh;
        struct ifaddrmsg ifa;
        char             attrbuf[128];
    } req;

    memset(&req, 0, sizeof(req));

    // Netlink header: we're adding a new address
    req.nlh.nlmsg_len   = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    req.nlh.nlmsg_type  = RTM_NEWADDR;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL;
    req.nlh.nlmsg_seq   = 1;

    // Interface address message: IPv4, /24 subnet
    req.ifa.ifa_family    = AF_INET;
    req.ifa.ifa_prefixlen = 24;  // /24 subnet
    req.ifa.ifa_index     = if_nametoindex(name);
    req.ifa.ifa_scope     = RT_SCOPE_UNIVERSE;

    // Convert IP string to binary format
    struct in_addr addr;
    if (inet_pton(AF_INET, ip, &addr) != 1) {
        close(sock);
        return TUN_INET_ERROR;
    }

    // Add the IP address as a Netlink attribute (IFA_LOCAL)
    struct rtattr *rta = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.nlh.nlmsg_len));
    rta->rta_type = IFA_LOCAL;
    rta->rta_len  = RTA_LENGTH(sizeof(addr));
    memcpy(RTA_DATA(rta), &addr, sizeof(addr));
    req.nlh.nlmsg_len = NLMSG_ALIGN(req.nlh.nlmsg_len) + RTA_LENGTH(sizeof(addr));
    
    // Send the message to the kernel
    if (send(sock, &req, req.nlh.nlmsg_len, 0) < 0) {
        close(sock);
        return TUN_SEND_ERROR;
    }

    close(sock);
    return 0;
}

int tun_alloc(char *name) {
    struct ifreq ifr;
    int fd, err;
    char *clonedev = "/dev/net/tun";

    // Open the TUN clone device
    if ((fd = open(clonedev, O_RDWR)) < 0) 
        return TUN_FILE_ERROR;

    // Request an L3 TUN device
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    // If name is provided, request that specific name
    if (*name)
        strncpy(ifr.ifr_name, name, IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
        close(fd);
        return TUN_KWRITE_ERROR;
    }

    // Write back the actual assigned name (if the kernel chose one)
    memset(name, 0, IFNAMSIZ);
    strcpy(name, ifr.ifr_name);

    return fd;
}

Tun *tun_init(char *name, char *ip) {
    int fd = tun_alloc(name);
    if (fd == TUN_FILE_ERROR) {
        fprintf(stderr, "[ERROR]: Could not open /dev/net/tun\n");
        return NULL; 
    }
    if (fd == TUN_KWRITE_ERROR) {
        fprintf(stderr, "[ERROR]: Could not create TUN device - \"%s\"\n", strerror(errno));
        return NULL;
    }

    Tun *tun = (Tun *) malloc(sizeof(Tun));
    if (tun == NULL) {
        fprintf(stderr, "[ERROR]: Could not allocate Tun structure\n");
        close(fd);
        return NULL;
    }

    tun->name = strdup(name);
    tun->ip = strdup(ip);
    tun->fd = fd;

    // Configure the interface: add the IP and bring it up
    int ip_add = tun_ip_add(name, ip), ip_wake = tun_ip_wake(name);
    if (ip_add == TUN_DSOCK_ERROR || ip_wake == TUN_DSOCK_ERROR) {
        fprintf(stderr, "[ERROR]: Could not open Netlink socket for configuration\n");
        tun_destroy(tun);
        return NULL;
    }
    if (ip_add == TUN_SEND_ERROR || ip_wake == TUN_SEND_ERROR) {
        fprintf(stderr, "[ERROR]: Could not send configuration to kernel\n");
        tun_destroy(tun);
        return NULL;
    }
    if (ip_add == TUN_INET_ERROR) {
        fprintf(stderr, "[ERROR]: Invalid IP address format: %s\n", ip);
        tun_destroy(tun);
        return NULL;
    }

    if (g_verbose)
        tun_print(tun);

    return tun;
}

void tun_destroy(Tun *tun) {
    if (tun == NULL) return;

    free(tun->name);
    free(tun->ip);
    close(tun->fd);
    free(tun);
    tun = NULL;
}

