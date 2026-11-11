#pragma once

#include "ip.hpp"
#include "netdevice.hpp"
#include <cstddef>
#include <cstdint>

struct icmp_header { // icmp header (network order)
  uint8_t type;      // discriminates the below union
  uint8_t code;      // info on the specific icmp message
  uint16_t checksum;

  union { // this union HAS to be 32-bit.
    struct {
      uint32_t __unused;
    } __attribute__((packed)) dest_unr; // TODO: RFC 4884.
    struct {
      uint16_t id;
      uint16_t sequence;
    } __attribute__((packed)) echo;
  };

  // Internet Header + 64 bits of Original Data Datagram
  uint8_t data[];

  static icmp_header *from_buffer(void *buf) {
    auto *hdr = reinterpret_cast<icmp_header *>(buf);
    return hdr;
  }
  static const icmp_header *from_buffer(const void *buf) {
    auto *hdr = reinterpret_cast<const icmp_header *>(buf);
    return hdr;
  }

  uint16_t calc_checksum(size_t len) const;
  void update_checksum(size_t len);
  bool checksum_ok(size_t len) const { return calc_checksum(len) == 0; }
} __attribute__((packed));

int handle_incoming_icmp(const ip_header *ip_hdr, size_t len, const netdevice *netdev);

#include <type_traits>
// ensure the icmp header has the correct size.
static_assert(sizeof(icmp_header) == 8);
