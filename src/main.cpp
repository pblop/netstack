#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <unistd.h>

#include "tap.hpp"

int main(int argc, char *argv[]) {
  char ifname[IFNAMSIZ];

  char *requested_ifname = argc > 1 ? argv[1] : NULL;
  int tap_fd = tap_connect(requested_ifname, ifname);
  if (tap_fd < 0) {
    fprintf(stderr, "Couldn't create TAP device\n");
    return 1;
  }
  printf("TAP device name: %s\n", ifname);

  // if (configure_iface(ifname, sinet_addr("192.168.234.2"),
  //                     sinet_addr("255.255.255.0")) < 0) {
  //   fprintf(stderr, "Couldn't configure the TUN interface.\n");
  //   close(tap_fd);
  //   return 1;
  // }
  //printf("TAP interface configured\n");

  // Use the actual TAP device.

  close(tap_fd);

  return 0;
}
