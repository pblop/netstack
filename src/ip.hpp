#pragma once

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
  uint16_t tot_len;
  uint16_t id;
  uint16_t frag_off; // flags (3 bits) + fragment offset (13 bits)
  uint8_t ttl;
  uint8_t protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;

  uint16_t calc_checksum() const;
  void update_checksum() { check = 0; check = calc_checksum(); }
  bool checksum_ok() const { return calc_checksum() == 0; }
} __attribute__((packed));

uint16_t ip_checksum(const void *data, size_t len);
