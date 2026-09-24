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

int tuntap_connect(char *ifname, char *ifname_out);

int configure_iface(char *ifname, in_addr_t addr, in_addr_t netmask);

// s-afer inet_addr.
in_addr_t sinet_addr(const char *ip_str);
