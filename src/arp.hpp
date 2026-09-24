#pragma once
#include "ether.hpp"
#include "netdevice.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <unordered_map>

struct arp_header {  // arp header (network order)
  uint16_t hwtype_;  // hardware address space
  uint16_t protype_; // protocol address space
  uint8_t hwsize;    // byte length of each hardware address
  uint8_t prosize;   // byte length of each protocol address
  uint16_t opcode_;  // opcode
  uint8_t data[];

  // a bunch other fields (ar_sha, ar_spa, ar_tha, ar_tpa) follow,
  // but they're variable length and they're not yet needed.

  static arp_header *from_buffer(uint8_t *buf) {
    auto *hdr = reinterpret_cast<arp_header *>(buf);
    return hdr;
  }

  uint16_t hwtype() const { return ntohs(hwtype_); };
  uint16_t protype() const { return ntohs(protype_); };
  uint16_t opcode() const { return ntohs(opcode_); };
  void set_hwtype(uint16_t v) { hwtype_ = htons(v); }
  void set_protype(uint16_t v) { protype_ = htons(v); }
  void set_opcode(uint16_t v) { opcode_ = htons(v); }

  std::array<char, 18> hrd_to_str() const {
    switch (hwtype()) {
    case ARPHRD_ETHER:
      return {"ETHER"};
    default:
      std::array<char, 18> buf;
      snprintf(buf.data(), sizeof(buf), "%d", hwtype());
      return buf;
    }
  }
  std::array<char, 18> pro_to_str() const {
    if (hwtype() == ARPHRD_ETHER) {
      switch (protype()) {
      case ETH_P_IP:
        return {"IPv4"};
      case ETH_P_IPV6:
        return {"IPv6"};
      default:
        std::array<char, 18> buf;
        snprintf(buf.data(), sizeof(buf), "%d", hwtype());
        return buf;
      }
    } else {
      std::array<char, 18> buf;
      snprintf(buf.data(), sizeof(buf), "%d", hwtype());
      return buf;
    }
  }
  std::array<char, 18> op_to_str() const {
    switch (opcode()) {
    case ARPOP_REQUEST:
      return {"REQUEST"};
    case ARPOP_REPLY:
      return {"REPLY"};
    default:
      std::array<char, 18> buf;
      snprintf(buf.data(), sizeof(buf), "%d", opcode());
      return buf;
    }
  }
} __attribute__((packed)); // tell clang to not add padding to the struct, so it
                           // matches the actual arp header layout in memory.

struct arp_ipv4 {
  unsigned char smac[6];
  uint32_t sip;
  unsigned char dmac[6];
  uint32_t dip;
} __attribute__((packed));

struct AddressResolutionModule {
  // Instead of having a <<protocol type, sender protocol address>, sender
  // hardware address> table, where the sender protocol address would vary in
  // size between protocols, I'll have one table per protocol.

  // the translation table for ipv4 <sender prot addr, sender hw addr>
  std::unordered_map<uint32_t, std::array<uint8_t, 6>> translation_table_ipv4;

  int handle_incoming_arp(eth_header *eth_hdr, size_t len, netdevice *netdev);
};
