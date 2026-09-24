#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/if_ether.h>

#include "tap.hpp"

void eth_addr_to_str(const uint8_t *addr, char *str, size_t str_size) {
  // high byte first, ethernet order.
  snprintf(str, str_size, "%02x:%02x:%02x:%02x:%02x:%02x",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

int main(int argc, char *argv[]) {
  char ifname[IFNAMSIZ];

  char *requested_ifname = argc > 1 ? argv[1] : NULL;
  int tap_fd = tap_connect(requested_ifname, ifname);
  if (tap_fd < 0) {
    fprintf(stderr, "Couldn't create TAP device\n");
    return 1;
  }
  printf("TAP device name: %s\n", ifname);

  if (configure_iface(ifname, sinet_addr("192.168.234.2"),
                      sinet_addr("255.255.255.0")) < 0) {
    fprintf(stderr, "Couldn't configure the TUN interface.\n");
    close(tap_fd);
    return 1;
  }
  printf("TAP interface configured\n");

  // Use the actual TAP device.
  while (1) {
    int err;
    struct ethhdr hdr;
    char src_str[18], dest_str[18];

    if ((err = read(tap_fd, &hdr, sizeof(hdr))) < 0) {
      fprintf(stderr, "Error reading from TAP device: %d\n", err);
    }
    eth_addr_to_str(hdr.h_source, src_str, sizeof(src_str));
    eth_addr_to_str(hdr.h_dest, dest_str, sizeof(dest_str));

    printf("Read %lu bytes from TAP device: %s, %s, %d\n", sizeof(hdr), src_str, dest_str, hdr.h_proto);
    fflush(stdout);
  }

  close(tap_fd);

  return 0;
}
