#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int tuntap_connect(char *ifname, short flags, char *ifname_out) {
  int tuntap_fd;
  struct ifreq ifr;

  // Open the TUN/TAP device
  if ((tuntap_fd = open("/dev/net/tun", O_RDWR | O_CLOEXEC)) < 0) {
    perror("Failed to open TUN device");
    return -1;
  }

  // Configure the TAP device
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;
  if (ifname != NULL) {
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  }

  // Try to create the device.
  if (ioctl(tuntap_fd, TUNSETIFF, (void *)&ifr) < 0) {
    if (errno == EPERM) {
      // tap0 is the default name for the first TAP device. We assume
      // the user hasn't created any other TAP devices and is trying
      // to use that default name. But we can't be sure. Thus, the info
      // message below might be wrong.
      const char *dev = ifname != NULL ? ifname : "tap0";
      perror("Failed to configure TUN device");
      if (ifname == NULL) {
        fprintf(stderr, "Couldn't create a TUN device. You may need to specify "
                        "a device name as the first argument.\n");
      } else {
        fprintf(stderr,
                "'%s' missing or not owned by you. "
                "Run `sudo ip tuntap add dev %s mode tap user $USER` "
                "or run this executable as root.\n",
                dev, dev);
      }
    } else {
      perror("Failed to configure TUN device");
    }

    close(tuntap_fd);
    return -1;
  }

  if (ifname_out != NULL) {
    strncpy(ifname_out, ifr.ifr_name, IFNAMSIZ);
  }
  return tuntap_fd;
}

int main(int argc, char *argv[]) {
  char ifname[IFNAMSIZ];
  short tuntap_flags =
      IFF_TAP | IFF_NO_PI; // TAP device without packet information

  char *requested_ifname = argc > 1 ? argv[1] : NULL;

  int tuntap_fd = tuntap_connect(requested_ifname, tuntap_flags, ifname);
  if (tuntap_fd < 0) {
    fprintf(stderr, "Couldn't create TAP device\n");
    return 1;
  }

  printf("TUN device name: %s\n", ifname);

  // You can now use the TUN device (tun_fd) to read and write network packets.

  // Remember to close the TUN device when done
  close(tuntap_fd);

  return 0;
}
