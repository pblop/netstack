#include "arp.hpp"
#include "ether.hpp"
#include "netdevice.hpp"

int AddressResolutionModule::handle_incoming_arp(eth_header *eth_hdr,
                                                 size_t len,
                                                 netdevice *netdev) {
  auto *arp_hdr = arp_header::from_buffer(eth_hdr->payload);
  printf("  (%5zub): hrd=%s, len=%d\n", len, arp_hdr->hrd_to_str().data(),
         arp_hdr->hwsize);
  printf("            pro=%s, len=%d\n", arp_hdr->pro_to_str().data(),
         arp_hdr->prosize);
  printf("            op =%s\n", arp_hdr->op_to_str().data());
  fflush(stdout);

  // ?Do I have the hardware type in ar$hrd?
  if (arp_hdr->hwtype() != ARPHRD_ETHER)
    return -1; // i don't know this hw type
  // TODO: [optionally check the hardware length ar$hln]

  // ?Do I speak the protocol in ar$pro?
  if (arp_hdr->protype() != ETH_P_IP)
    return -2; // i don't know the protocol
  // TODO: [optionally check the protocol length ar$pln]

  // Merge_flag := false
  bool merge_flag = false;

  auto arp_data = reinterpret_cast<arp_ipv4 *>(arp_hdr->data);

  // If the pair <protocol type, sender protocol address> is already in my
  // translation table, update the sender hardware address field of the entry
  // with the new information in the packet and set Merge_flag to true.
  auto it = translation_table_ipv4.find(arp_data->sip);
  if (it != translation_table_ipv4.end()) {
    it->second = std::to_array(arp_data->smac);
    merge_flag = true;
  }

  // ?Am I the target protocol address?
  if (arp_data->dip != netdev->ipv4addr)
    return -3; // not my packet to handle

  // If Merge_flag is false, add the triplet <protocol type, sender protocol
  // address, sender hardware address> to the translation table.
  if (!merge_flag)
    translation_table_ipv4[arp_data->sip] = std::to_array(arp_data->smac);

  // ?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)
  if (arp_hdr->opcode() != ARPOP_REQUEST)
    return -4; // not a request for me to handle.

  // Swap hardware and protocol fields, putting the local hardware and protocol
  // addresses in the sender fields.
  uint16_t tmp = arp_data->sip; // cannot use std::swap, as sip and dip are
                                // misaligned (the arp_header struct is packed).
  arp_data->sip = arp_data->dip;
  arp_data->dip = tmp;
  std::swap(arp_data->smac, arp_data->dmac);

  // Set the ar$op field to ares_op$REPLY.
  arp_hdr->set_opcode(ARPOP_REPLY);

  // Send the packet to the (new) target hardware address on the same hardware
  // on which the request was received.
  size_t payload_len = sizeof(arp_header) + sizeof(arp_ipv4);
  netdev->transmit(eth_hdr, payload_len, ETH_P_ARP, arp_data->dmac);
  return 0;
}
