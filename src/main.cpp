#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <unistd.h>

#include "arp.hpp"
#include "ether.hpp"
#include "netdevice.hpp"
#include "tap.hpp"

void handle_eth_frame(eth_header *hdr, size_t len);
void handle_arp_packet(arp_header *hdr, size_t len);

AddressResolutionModule arp_module;
netdevice netdev;

void handle_eth_frame(eth_header *hdr, size_t len) {
  char src_str[18], dest_str[18];
  hdr->saddr_to_str(src_str, sizeof(src_str));
  hdr->daddr_to_str(dest_str, sizeof(dest_str));

  printf("(%5zu bytes) %s -> %s\n", len, src_str, dest_str);
  if (hdr->is_type()) {
    switch (hdr->type_len) {
    case ETH_P_ARP: {
      printf("  Type: ARP\n");
      arp_module.handle_incoming_arp(hdr, len - sizeof(eth_header), &netdev);
      break;
    }
    case ETH_P_IPV6:
      printf("  Type: IPv6\n");
      break;
    default:
      printf("  Unknown type 0x%x\n", hdr->type_len);
      break;
    }
  } else {
    printf("  Length: %d\n", hdr->type_len);
  }
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  TapDevice tap;

  char *requested_ifname = argc > 1 ? argv[1] : NULL;
  if (tap.connect(requested_ifname) < 0) {
    fprintf(stderr, "Couldn't create TAP device\n");
    return 1;
  }
  printf("TAP device name: %s\n", tap.ifname);

  if (tap.up_iface() < 0) {
    fprintf(stderr, "Couldn't bring the TAP interface up.\n");
    tap.close();
    return 1;
  }
  printf("TAP interface configured\n");

  netdev.ipv4_addr = sinet_addr("10.0.0.2");

  // Use the actual TAP device.
  while (1) {
    int err;
    uint8_t buf[ETH_FRAME_LEN];

    if ((err = tap.read(buf, sizeof(buf))) < 0) {
      fprintf(stderr, "Error reading from TAP device: %d\n", err);
    }
    auto *hdr = eth_header::from_buffer(buf);
    handle_eth_frame(hdr, err);
  }

  tap.close();

  return 0;
}
