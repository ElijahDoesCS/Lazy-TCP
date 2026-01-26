#include "./v_device.h"

int tun_alloc(char *dev, int flags) {
  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  // Open the clone device (open an endpoint in the kernel, request creation)
  if ((fd = open(clonedev, O_RDWR)) < 0 ) return fd;

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags; // IFF_TUN

  if (*dev) // Pick a device name, or pick the next unallocated device name
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  // Try to create the device (invoke the driver to create a new instance, bind to fd)
  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
    close(fd);
    return err;
  }

  // Write back the name of the virtual interface on success. Caller must allocate space
  strcpy(dev, ifr.ifr_name);

  // Return the special file descriptor used to talk to the virtual device
  return fd;
}

// Initialize a tun or tap device to an IP
int tun_init(bool verbose, Virtual_Device *vd) {
  char *ip = vd->ip;
  char *dev_name = vd->dev;

  // Configure the TUN device at localhost
  char cmd[256];
  sprintf(cmd, "ip addr add %s dev %s", ip, dev_name);
  if (system(cmd) == -1)
    return 1;

  sprintf(cmd, "ip link set %s up", dev_name);
  if (system(cmd) == -1)
    return 1;
  
  if (verbose)
    printf("TUN interface %s is up at %s\n", ip, dev_name);

  return 0;
}

void virtual_device_print(Virtual_Device *vd) {
  printf("Virtual device:\n");
  printf("    IP             : %s\n", vd->ip);
  printf("    Device name    : %s\n", vd->dev);
  printf("    File desciptor : %d\n", vd->fd);
}

void virtual_device_destroy(Virtual_Device vd) {
  close(vd.fd);
  free(vd.ip);
  free(vd.dev);
}