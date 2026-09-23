#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <unistd.h>

#include "tap.hpp"

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

  if (configure_iface(ifname, sinet_addr("192.168.234.2"),
                      sinet_addr("255.255.255.0")) < 0) {
    fprintf(stderr, "Couldn't configure the TUN interface.\n");
    close(tuntap_fd);
    return 1;
  }
  printf("TUN interface configured\n");

  // You can now use the TUN device (tun_fd) to read and write network packets.

  close(tuntap_fd);

  return 0;
}
