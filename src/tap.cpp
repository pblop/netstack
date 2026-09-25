#include "tap.hpp"

int TapDevice::connect(char *ifname_in) {
  int tuntap_fd;
  struct ifreq ifr;

  // Open the TUN/TAP device
  if ((tuntap_fd = open("/dev/net/tun", O_RDWR | O_CLOEXEC)) < 0) {
    perror("Failed to open TUN device");
    return -1;
  }

  // Configure the TAP device
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI; // TAP device without packet information
  if (ifname_in != NULL) {
    strncpy(ifr.ifr_name, ifname_in, IFNAMSIZ);
  }

  // Try to create the device.
  if (ioctl(tuntap_fd, TUNSETIFF, (void *)&ifr) < 0) {
    if (errno == EPERM) {
      // tap0 is the default name for the first TAP device. We assume
      // the user hasn't created any other TAP devices and is trying
      // to use that default name. But we can't be sure. Thus, the info
      // message below might be wrong.
      const char *dev = ifname_in != NULL ? ifname_in : "tap0";
      perror("Failed to configure TUN device");
      if (ifname_in == NULL) {
        fprintf(stderr, "Couldn't create a TUN device. You may need to specify "
                        "a device name as the first argument.\n");
      } else {
        fprintf(stderr,
                "'%s' missing or not owned by you. "
                "Run `sudo ip tuntap add dev %s mode tap user $USER` "
                "or run this executable as root.\n",
                dev, dev);
      }
    } else {
      perror("Failed to configure TUN device");
    }

    this->close();
    return -1;
  }

  strncpy(this->ifname, ifr.ifr_name, IFNAMSIZ);
  this->fd = tuntap_fd;
  return tuntap_fd;
}

int TapDevice::up_iface() {
  int sock_fd;
  struct ifreq ifr;

  // Create a channel into the NET kernel
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0) {
    perror("Failed to create configuration socket");
    return -1;
  }

  // Prepare the ifreq structure with the interface name (to set interface
  // values).
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, this->ifname, IFNAMSIZ);
  
  // Bring the interface up.
  if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) {
    perror("Failed to get interface flags");
    ::close(sock_fd);
    return -1;
  }
  // If the interface is not up and running, we need to set the flags to bring
  // it up.
  if (!(ifr.ifr_flags & IFF_UP && ifr.ifr_flags & IFF_RUNNING)) {
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

    if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0) {
      perror("Failed to set interface flags");
      ::close(sock_fd);
      return -1;
    }
  }

  ::close(sock_fd);
  return 0;
}

// This doesn't use Netlink on purpose. That API is a bit more complex, and I'm
// just looking for a simple way to set an IP address on the interface.
int TapDevice::configure_iface(in_addr_t addr, in_addr_t netmask) {
  int sock_fd;
  struct ifreq ifr;
  struct sockaddr_in sai;
  struct ifreq ifr_read;
  struct sockaddr_in *sai_read;

  // Create a channel into the NET kernel
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0) {
    perror("Failed to create configuration socket");
    return -1;
  }

  // Before we set the IP address, netmask and flags (which we need to have
  // permission for), we check if the interface already has the correct values.
  // If it does, we can skip the configuration (and avoid needing to run as
  // root). So we need to prepare the ifreq structure to read the interface
  // values onto it.
  memset(&ifr_read, 0, sizeof(ifr_read));
  strncpy(ifr_read.ifr_name, this->ifname, IFNAMSIZ);

  // Prepare the ifreq structure with the interface name (to set interface
  // values).
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, this->ifname, IFNAMSIZ);

  // Get the current IP address of the interface to check if it matches the
  // desired address.
  // The kernel returns EADDRNOTAVAIL if the interface has no IPv4 address yet,
  // which just means we have to set it.
  bool has_addr = true;
  if (ioctl(sock_fd, SIOCGIFADDR, &ifr_read) < 0) {
    if (errno != EADDRNOTAVAIL) {
      perror("Failed to get interface address");
      ::close(sock_fd);
      return -1;
    }
    has_addr = false;
  }
  sai_read = reinterpret_cast<struct sockaddr_in *>(&ifr_read.ifr_addr);

  // If the interface doesn't have the correct IP address, we need to set it.
  if (!has_addr || sai_read->sin_family != AF_INET || sai_read->sin_port != 0 ||
      sai_read->sin_addr.s_addr != addr) {
    // Configure the IP address for the interface.
    // Prepare the sockaddr_in structure with the desired IP address.
    memset(&sai, 0, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = 0;
    sai.sin_addr.s_addr = addr;
    // Copy it into the ifreq structure and set the IP address.
    memcpy(&ifr.ifr_addr, &sai, sizeof(sai));
    if (ioctl(sock_fd, SIOCSIFADDR, &ifr) < 0) {
      perror("Failed to set IP address");
      ::close(sock_fd);
      return -1;
    }
  }

  if (ioctl(sock_fd, SIOCGIFNETMASK, &ifr_read) < 0) {
    perror("Failed to get interface netmask");
    ::close(sock_fd);
    return -1;
  }
  // no need to reinterpret_cast here, since we already did it above and
  // sai_read is still valid (poins to stack memory).

  // If the interface doesn't have the correct netmask, we need to set it.
  if (sai_read->sin_addr.s_addr != netmask) {
    // Configure the netmask for the interface (reusing the ifreq and
    // sockaddr_in structures).
    sai.sin_addr.s_addr = netmask;
    memcpy(&ifr.ifr_netmask, &sai, sizeof(sai));
    if (ioctl(sock_fd, SIOCSIFNETMASK, &ifr) < 0) {
      perror("Failed to set netmask");
      ::close(sock_fd);
      return -1;
    }
  }

  // Bring the interface up.
  if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0) {
    perror("Failed to get interface flags");
    ::close(sock_fd);
    return -1;
  }
  // If the interface is not up and running, we need to set the flags to bring
  // it up.
  if (!(ifr.ifr_flags & IFF_UP && ifr.ifr_flags & IFF_RUNNING)) {
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

    if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0) {
      perror("Failed to set interface flags");
      ::close(sock_fd);
      return -1;
    }
  }

  ::close(sock_fd);
  return 0;
}

// TODO: maybe instead of exiting make use of C++'s magnificent error handling
// and blow up the stack with an exception.
in_addr_t sinet_addr(const char *ip_str) {
  struct in_addr addr;
  if (inet_aton(ip_str, &addr) != 1) {
    fprintf(stderr, "Invalid IP address: %s\n", ip_str);
    exit(EXIT_FAILURE);
  }

  return addr.s_addr;
}

int TapDevice::read(uint8_t *buf, size_t len) {
  int bytes_read = ::read(fd, buf, len);
  if (bytes_read < 0) {
    perror("Failed to read from TAP device");
    return -1;
  }
  return bytes_read;
}

int TapDevice::write(uint8_t *buf, size_t len) {
  int bytes_written = ::write(fd, buf, len);
  if (bytes_written < 0) {
    perror("Failed to write to TAP device");
    return -1;
  }
  return bytes_written;
}

int TapDevice::close() {
  if (fd >= 0) {
    int ret = ::close(fd);
    if (ret < 0) {
      perror("Failed to close TAP device");
      return -1;
    }
    fd = -1;
  }
  return 0;
}
