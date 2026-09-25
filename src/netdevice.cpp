#include "netdevice.hpp"
#include "ether.hpp"

int netdevice::transmit(eth_header *header, size_t payload_len,
                        uint16_t ethertype, const uint8_t dst[6]) {
  // Set the source and destination fields in the ETH header.
  memcpy(header->saddr, this->hwaddr, sizeof(header->saddr));
  memcpy(header->daddr, dst, sizeof(header->daddr));

  header->set_type_len(ethertype);

  // Pad frame to 64 bytes. That includes FCS (4 bytes), which we don't need
  // with TAP. So we're padding to 60 bytes (for completeness, although TAP
  // doesn't require this padding).
  ssize_t pad = 60 - sizeof(eth_header) - payload_len;
  if (pad > 0) {
    auto pad_start =
        reinterpret_cast<uint8_t *>(header) + sizeof(eth_header) + payload_len;
    memset(pad_start, 0, pad);
  } else {
    pad = 0; // for calculating later, we didn't add any padding.
  }

  return tap->write(reinterpret_cast<uint8_t *>(header),
             sizeof(eth_header) + payload_len + pad);
}
