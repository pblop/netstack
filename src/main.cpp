#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <unistd.h>

#include "ether.hpp"
#include "tap.hpp"

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
    auto *hdr = eth_header::from_buffer(buf);
    hdr->saddr_to_str(src_str, sizeof(src_str));
    hdr->daddr_to_str(dest_str, sizeof(dest_str));

    printf("(%lub) %s -> %s. %s: %d\n", sizeof(buf), src_str, dest_str,
           hdr->is_type() ? "Type" : "Length", hdr->type_len);
    fflush(stdout);
  }

  tap.close();

  return 0;
}
