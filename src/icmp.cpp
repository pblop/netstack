#include "icmp.hpp"
#include "ip.hpp"

uint16_t icmp_header::calc_checksum(size_t len) const {
  return ip_checksum(this, len);
}
void icmp_header::update_checksum(size_t len) {
  checksum = 0;
  checksum = calc_checksum(len);
}
