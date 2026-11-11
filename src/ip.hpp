#pragma once

#include "netdevice.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

struct ip_header { // network order
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint8_t ihl : 4; // the number of 32-bit words in the header (counting options).
  uint8_t version : 4;
#else
  uint8_t version : 4;
  uint8_t ihl : 4;
#endif
  uint8_t tos;
  uint16_t tot_len_;
  uint16_t id;
  uint16_t frag_off; // flags (3 bits) + fragment offset (13 bits)
  uint8_t ttl;
  uint8_t protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;

  static ip_header *from_buffer(uint8_t *buf) {
    auto *hdr = reinterpret_cast<ip_header *>(buf);
    return hdr;
  }

  uint16_t calc_checksum() const;
  void update_checksum() { check = 0; check = calc_checksum(); }
  bool checksum_ok() const { return calc_checksum() == 0; }

  uint16_t tot_len() const { return ntohs(tot_len_); }

  // ihl but in bytes.
  size_t header_len() const { return ihl * 4; }
  // options + padding (which is empty when ihl == 5)
  uint8_t *options() {
    return reinterpret_cast<uint8_t *>(this) + sizeof(ip_header); 
  }
  uint8_t *payload() {
    return reinterpret_cast<uint8_t *>(this) + header_len(); 
  }
  const uint8_t *payload() const {
    return reinterpret_cast<const uint8_t *>(this) + header_len(); 
  }
  size_t payload_len() const { return tot_len() - header_len(); }
} __attribute__((packed));

uint16_t ip_checksum(const void *data, size_t len);

struct IpModule {
  int handle_incoming_ip(eth_header *eth_hdr, size_t len, netdevice *netdev);
};

constexpr std::array<char, INET_ADDRSTRLEN> inet_ntoy(uint32_t addr) {
  std::array<char, INET_ADDRSTRLEN> str{};
  inet_ntop(AF_INET, &addr, str.data(), str.size());
  return str;
}
