#include "ip.hpp"
#include <cstring>
#include <gtest/gtest.h>

namespace {
// The example IP packet from Wikipedia (checksum between brackets, first line
// is header).
// 4500 0073 0000 4000 4011 [b861] c0a8 0001 c0a8 00c7
// 0035 e97c 005f 279f 1e4b 8180
constexpr unsigned char ip_packet[32] = {
    // header
    0x45, 0x00, 0x00, 0x73, 0x00, 0x00, 0x40, 0x00, 0x40, 0x11, 0xb8, 0x61,
    0xc0, 0xa8, 0x00, 0x01, 0xc0, 0xa8, 0x00, 0xc7,
    // rest
    0x00, 0x35, 0xe9, 0x7c, 0x00, 0x5f, 0x27, 0x9f, 0x1e, 0x4b, 0x81, 0x80};
} // namespace

TEST(IpChecksum, ValidHeadersCalculatedChecksumIsZero) {
  unsigned char buf[32];
  std::memcpy(buf, ip_packet, sizeof(buf));

  auto *h = reinterpret_cast<ip_header *>(buf);
  EXPECT_EQ(h->calc_checksum(), 0);
}

TEST(IpChecksum, ChecksumIsCorrectlyCalculated) {
  unsigned char buf[32];
  std::memcpy(buf, ip_packet, sizeof(buf));
  // garbage in check field
  buf[10] = 0xde;
  buf[11] = 0xad;

  auto *h = reinterpret_cast<ip_header *>(buf);
  h->update_checksum();
  EXPECT_EQ(buf[10], 0xb8); // byte compare: endian-independent
  EXPECT_EQ(buf[11], 0x61);
}

// Check whether the source being loaded at an odd address causes UB (picked up
// by UBSan).
TEST(IpChecksum, OddAddress) {
  alignas(4) unsigned char buf[sizeof(ip_packet) + 1];
  std::memcpy(buf + 1, ip_packet, sizeof(ip_packet));

  auto *h = reinterpret_cast<ip_header *>(buf + 1);
  EXPECT_EQ(h->calc_checksum(), 0); // UBSan flags misaligned uint16_t load
}
