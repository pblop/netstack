#include "ip.hpp"

uint16_t ip_header::calc_checksum() const {
  // Compute Internet Checksum for "count" bytes beginning at location "addr".
  // Original from RFC 1071 https://tools.ietf.org/html/rfc1071

  uint32_t sum = 0;
  size_t count = ihl * 4; // 32-bit words are 4 bytes
  const uint16_t *ptr = reinterpret_cast<const uint16_t *>(this);

  while (count > 1) {
    /*  This is the inner loop */
    sum += *ptr++;
    count -= 2;
  }

  /*  Add left-over byte, if any */
  if (count > 0)
    sum += *reinterpret_cast<const uint8_t *>(ptr);

  /*  Fold 32-bit sum to 16 bits */
  while (sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  return ~sum;
}
