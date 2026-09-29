#include "ip.hpp"
#include <cstring>

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
  size_t count = ihl * 4; // 32-bit words are 4 bytes
  return ip_checksum(this, count);
}
