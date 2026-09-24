#pragma once

#include <cstddef>
#include <cstdint>
#include <linux/if_ether.h>

#include <unistd.h>

struct eth_header { // ethernet header (big endian)
  uint8_t daddr[6]; // destination MAC address
  uint8_t saddr[6]; // source MAC address
  // If type_len <= 0x05DC (1500) => length
  // If type_len >= 0x0600 (1536) => type
  uint16_t type_len; // the type/length field
  // The payload field is padded if its length is less than the minimum
  // ethernet frame size (48 bytes). The padding is not included in the type_len
  // field.
  uint8_t payload[];
} __attribute__((packed)); // tell clang to not add padding to the struct, so it
                           // matches the actual ethernet header layout in
                           // memory.
