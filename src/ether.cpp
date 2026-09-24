#include "ether.hpp"

void eth_addr_to_str(const uint8_t *addr, char *str, size_t str_size) {
  // high byte first, ethernet order.
  snprintf(str, str_size, "%02x:%02x:%02x:%02x:%02x:%02x",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}
