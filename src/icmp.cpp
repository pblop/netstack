#include "icmp.hpp"
#include <linux/icmp.h>

uint16_t icmp_header::calc_checksum(size_t len) const {
  return ip_checksum(this, len);
}
void icmp_header::update_checksum(size_t len) {
  checksum = 0;
  checksum = calc_checksum(len);
}

int handle_incoming_icmp(const ip_header *ip_hdr, size_t len, const netdevice *netdev) {
  const auto *icmp_hdr = icmp_header::from_buffer(ip_hdr->payload());

  printf("    (%5zub): type=%d\n", len, icmp_hdr->type);
  fflush(stdout);

  if (!icmp_hdr->checksum_ok(len)) return 1;

  switch (icmp_hdr->type) {
    case ICMP_ECHO: 
      printf("      Type: ECHO\n");
      break;
    default:
      printf("      Unknown ICMP type %d\n", icmp_hdr->type);
      break;
  }

  return 0;
  
}
