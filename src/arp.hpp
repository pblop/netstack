#pragma once
#include <cstdint>
#include <netinet/in.h>
#include <linux/if_arp.h>

struct arp_header {
	uint16_t ar_hrd; // hardware address space
	uint16_t ar_pro; // protocol address space
	uint8_t	 ar_hln; // byte length of each hardware address
	uint8_t	 ar_pln; // byte length of each protocol address
	uint16_t ar_op;  // opcode

	// a bunch other fields (ar_sha, ar_spa, ar_tha, ar_tpa) follow,
	// but they're variable length and they're not yet needed.
	
	static arp_header *from_buffer(uint8_t *buf) {
		auto *hdr = reinterpret_cast<arp_header *>(buf);
		hdr->ar_hrd = ntohs(hdr->ar_hrd);
		hdr->ar_pro = ntohs(hdr->ar_pro);
		hdr->ar_op = ntohs(hdr->ar_op);
		return hdr;
	}
};
