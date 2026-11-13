#include "ip.hpp"
#include "ether.hpp"
#include "icmp.hpp"
#include <cassert>
#include <cstring>
#include <netinet/in.h>

uint16_t ip_checksum(const void *data, size_t len) {
  // Compute Internet Checksum for "count" bytes beginning at location "addr".
  // Original from RFC 1071 https://tools.ietf.org/html/rfc1071.
  // Modified to be more C++ friendly (old one had UB I believe, see
  // tests/ip_checksum.cpp's OddAddress test).

  uint32_t sum = 0;
  const auto *bytes = reinterpret_cast<const std::byte *>(data);

  // while( count > 1 ){
  //   /*  This is the inner loop */
  //   sum += * (unsigned short) addr++;
  //   count -= 2;
  // }
  // Instead of the above loop that uses dereferencing to read a short at a
  // time, we copy the short with memcpy and then operate on it. This removes
  // the UB on reads if the short isn't correctly aligned (which could be, given
  // that every structure above the ip_header is packed).
  for (size_t i = 0; i + 1 < len; i += 2) {
    uint16_t word;
    std::memcpy(&word, bytes + i, sizeof word);
    sum += word;
  }

  /*  Add left-over byte, if any */
  if (len % 2 == 1) {
    // This is the original code
    //sum += * (unsigned char *) addr;
    uint16_t byte = *reinterpret_cast<const unsigned char *>(bytes+len-1);
    // This fixes the results on a big endian machine, I think.
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    byte <<= 8;
    #endif
    sum += byte;
  }

  /*  Fold 32-bit sum to 16 bits */
  while (sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  return ~sum;
}

uint16_t ip_header::calc_checksum() const {
  return ip_checksum(this, header_len());
}

constexpr uint32_t ipv4host(const uint32_t ipv4addr, const uint32_t ipv4mask) {
  return ipv4addr & ~ipv4mask;
}

int IpModule::handle_incoming_ip(eth_header *eth_hdr, size_t len,
                                 netdevice *netdev) {
  // return values:
  // = 0 - ok
  // > 0 - silently discard
  // < 0 - send icmp.
  
  auto *ip_hdr = ip_header::from_buffer(eth_hdr->payload);

  printf("  (%5zub):  src=%s, dst=%s\n", len, 
         inet_ntoy(ip_hdr->saddr).data(), inet_ntoy(ip_hdr->daddr).data());
  fflush(stdout);

  // text from rfc1122
  // verifies that the datagram is correctly formatted;
  if (ip_hdr->version != 4) return 1;
  if (!ip_hdr->checksum_ok()) return 2;
  if (ip_hdr->tot_len() != len) return 3; // ensure the actual and specified len 
                                          // are the same.

  // verifies that it is destined to the local host;
  // A host MUST silently discard an incoming datagram that is not destined for
  // the host.  An incoming datagram is destined for the host if the datagram's
  // destination address field is:
  if (ip_hdr->daddr != netdev->ipv4addr && // (1) (one of) the host's IP address(es)
      // (2) an IP broadcast address valid for the connected network \/
      // (a) this host on this network <- i believe i shouldn't answer that
      // (b) specified host in network <- rfc1700 says this should only be a
      // source address (page 4).
      ip_hdr->daddr != 0xffffffff &&       // (c) limited broadcast
      // no network broadcast (d). we don't do classed addresses 🙅‍♀️
      ip_hdr->daddr != netdev->ipv4addr)   // (e) subnet broadcast
      // no network broadcast (f)
      // (3) the address for a multicast group of which the host is a member on
      // the incoming physical interface. # TODO: no IGMP (yet)
    return 4;

  // TODO: id is ignored as of yet.
  // TODO: tos is ignored as of yet.

  // processes options;
  // TODO: options are ignored as of yet.

  // reassembles the datagram if necessary; and
  // passes the encapsulated message to the appropriate transport-layer protocol
  // module.
  // TODO: fragmentation/reassembly not supported as of yet.


  switch (ip_hdr->protocol) {
    case IPPROTO_ICMP: 
      printf("    Proto: ICMP\n");
      handle_incoming_icmp(ip_hdr, ip_hdr->payload_len(), netdev, this);
      break;
    default:
      printf("    Unknown proto 0x%x\n", ip_hdr->protocol);
      break;
  }

  return 0;
}

int IpModule::send_ip(uint32_t daddr, uint8_t proto, void *payload,
                      size_t payload_len, uint8_t ttl, uint32_t saddr) {
  // Create the IP packet. A full ethernet+ip packet has a max size, so all of
  // this can be created in the stack.
  static constexpr size_t HEADERSIZE = sizeof(eth_header) + sizeof(ip_header);
  if (payload_len > ETH_FRAME_LEN - HEADERSIZE) return -1;

  uint8_t buf[ETH_FRAME_LEN];
  auto *eth_hdr = eth_header::from_buffer(buf);
  auto *ip_hdr  = ip_header::from_buffer(eth_hdr->payload);
  size_t ip_len = sizeof(ip_header) + payload_len;

  ip_hdr->ihl = 5; // no options for now.
  ip_hdr->tos = 0;
  ip_hdr->set_tot_len(sizeof(ip_header) + payload_len);
  ip_hdr->set_id(id++); // this wraps around automatically.
  ip_hdr->frag_off = 0;

  // A host MUST NOT send a datagram with a Time-to-Live (TTL)
  // value of zero.
  if (ttl == 0) {
    fprintf(stderr, "TTL was 0 when sending IP packet\n");
    return -2;
  }
  ip_hdr->ttl = ttl;

  ip_hdr->protocol = proto;
  ip_hdr->daddr = daddr;

  // Get which network device is responsible for this IP address.
  auto dev = route(daddr);
  ip_hdr->saddr = dev->ipv4addr;
  ip_hdr->update_checksum();

  memcpy(ip_hdr->payload(), payload, payload_len);

  if (!arp->translation_table_ipv4.contains(daddr)) {
    fprintf(stderr, "ARP table doesn't have hw addr for %s\n",
            inet_ntoy(daddr).data());
    return -4;
  }

  std::array<uint8_t, 6> hw_addr = arp->translation_table_ipv4.at(daddr);
  return dev->transmit(eth_hdr, ip_len, ETH_P_IP, hw_addr.data());
}
