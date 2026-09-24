#pragma once

#include "ether.hpp"
#include "tap.hpp"
#include <cstdint>

struct netdevice {
  uint32_t ipv4addr;
  uint8_t hwaddr[6];
  TapDevice *tap;

  int transmit(eth_header* header, size_t payload_len, uint16_t ethertype, const uint8_t dst[8]);
};
