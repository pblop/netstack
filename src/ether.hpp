#pragma once

#include <cstddef>
#include <cstdint>
#include <linux/if_ether.h>

#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

void eth_addr_to_str(const uint8_t *addr, char *str, size_t str_size);

struct eth_header { // ethernet header (network order)
  uint8_t daddr[6]; // destination MAC address
  uint8_t saddr[6]; // source MAC address
  // If type_len <= 0x05DC (1500) => length
  // If type_len >= 0x0600 (1536) => type
  uint16_t type_len_; // the type/length field
  // The payload field is padded if its length is less than the minimum
  // ethernet frame size (48 bytes). The padding is not included in the type_len
  // field.
  uint8_t payload[];

  static eth_header *from_buffer(uint8_t *buf) {
    auto *hdr = reinterpret_cast<eth_header *>(buf);
    return hdr;
  }

  uint16_t type_len() const { return ntohs(type_len_); }
  void set_type_len(uint16_t v) { type_len_ = htons(v); }

  int is_type() const { return type_len() >= 0x0600; }
  int is_length() const { return type_len() <= 0x05DC; }
  void daddr_to_str(char *str, size_t str_size) const {
    eth_addr_to_str(daddr, str, str_size);
  }
  void saddr_to_str(char *str, size_t str_size) const {
    eth_addr_to_str(daddr, str, str_size);
  }
} __attribute__((packed)); // tell clang to not add padding to the struct, so it
                           // matches the actual ethernet header layout in
                           // memory.

#include <type_traits>
// ensure the struct is packed and has the correct size and layout.
static_assert(sizeof(eth_header) == 14);
static_assert(std::is_standard_layout_v<eth_header>);
static_assert(std::is_trivially_copyable_v<eth_header>);
