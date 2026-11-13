#include "icmp.hpp"
#include "ip.hpp"
#include <cstdio>
#include <linux/icmp.h>

uint16_t icmp_header::calc_checksum(size_t len) const {
  return ip_checksum(this, len);
}
void icmp_header::update_checksum(size_t len) {
  checksum = 0;
  checksum = calc_checksum(len);
}

int handle_incoming_icmp(ip_header *ip_hdr, size_t len, netdevice *netdev,
                         IpModule *ipm) {
  auto *icmp_hdr = icmp_header::from_buffer(ip_hdr->payload());

  printf("    (%5zub): type=%d\n", len, icmp_hdr->type);
  fflush(stdout);

  if (!icmp_hdr->checksum_ok(len))
    return 1;

  switch (icmp_hdr->type) {
  case ICMP_ECHO: {
    printf("      Type: ECHO\n");
    icmp_hdr->type = ICMP_ECHOREPLY;

    // The IP source address in an ICMP Echo Reply MUST be the same
    // as the specific-destination address (defined in Section
    // 3.2.1.3) of the corresponding ICMP Echo Request message.
    uint32_t saddr =
        ip_hdr->daddr == netdev->ipv4addr ? ip_hdr->daddr : netdev->ipv4addr;

    icmp_hdr->update_checksum(len);
      
    // TODO:
    // If a Source Route option is received in an ICMP Echo
    // Request, the return route MUST be reversed and used as a
    // Source Route option for the Echo Reply message.
    int ret = ipm->send_ip(ip_hdr->saddr, IPPROTO_ICMP, icmp_hdr, len, 64, saddr);
    printf("Returned %d\n", ret);
    break;
  }
  default:
    printf("      Unknown ICMP type %d\n", icmp_hdr->type);
    break;
  }

  return 0;
}
