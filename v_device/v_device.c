#include "./v_device.h"

int tun_alloc(char *dev, int flags) {
  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  // Open a clone device in the kernel
  if ((fd = open(clonedev, O_RDWR)) < 0 ) return fd;

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags; // IFF_TUN

  if (*dev) // Copy the name
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  // Invoke kernel driver to create new device
  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
    close(fd);
    return err;
  }

  // Caller must allocate space
  strcpy(dev, ifr.ifr_name);

  // Return the fd used to talk to the device
  return fd;
}

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

void virtual_device_destroy(Virtual_Device vd) {
  close(vd.fd);
  free(vd.ip);
  free(vd.dev);
}