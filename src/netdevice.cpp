#include "netdevice.hpp"

int netdevice::transmit(eth_header *header, size_t payload_len,
                        uint16_t ethertype, const uint8_t dst[8]) {
  return 0;
}
