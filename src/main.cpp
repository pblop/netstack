#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/if_ether.h>

#include "ether.hpp"
#include "tap.hpp"

void eth_addr_to_str(const uint8_t *addr, char *str, size_t str_size) {
  // high byte first, ethernet order.
  snprintf(str, str_size, "%02x:%02x:%02x:%02x:%02x:%02x",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

int main(int argc, char *argv[]) {
  TapDevice tap;

  char *requested_ifname = argc > 1 ? argv[1] : NULL;
  if (tap.connect(requested_ifname) < 0) {
    fprintf(stderr, "Couldn't create TAP device\n");
    return 1;
  }
  printf("TAP device name: %s\n", tap.ifname);

  if (tap.configure_iface(sinet_addr("192.168.234.2"),
                      sinet_addr("255.255.255.0")) < 0) {
    fprintf(stderr, "Couldn't configure the TUN interface.\n");
    tap.close();
    return 1;
  }
  printf("TAP interface configured\n");

  // Use the actual TAP device.
  while (1) {
    int err;
    uint8_t buf[ETH_FRAME_LEN];
    char src_str[18], dest_str[18];

    if ((err = tap.read(buf, sizeof(buf))) < 0) {
      fprintf(stderr, "Error reading from TAP device: %d\n", err);
    }
    struct eth_header *hdr = (struct eth_header *)buf;
    eth_addr_to_str(hdr->saddr, src_str, sizeof(src_str));
    eth_addr_to_str(hdr->daddr, dest_str, sizeof(dest_str));

    printf("(%lu) %s -> %s. Type: %d\n", sizeof(buf), src_str, dest_str, hdr->type_len);
    fflush(stdout);
  }

  tap.close();

  return 0;
}
