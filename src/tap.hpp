#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

struct TapDevice {
  int fd = -1;
  char ifname[IFNAMSIZ];

  int connect(char *ifname);
  int read(uint8_t *buf, size_t len);

  int configure_iface(in_addr_t addr, in_addr_t netmask);
  int close();
};


// s-afer inet_addr.
in_addr_t sinet_addr(const char *ip_str);
