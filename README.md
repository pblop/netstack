# netstack

A small network stack built from zero in C++ to help me understand how
networking works.

## Nix VM setup 

My machine runs macOS, and it [doesn't include a TAP device](https://tunnelblick.net/cTunTapConnections.html).
Because I want to build layer 2 and up, I need a TAP device to send and receive
Ethernet frames. I could use a TAP driver for macOS, but I think it'd be better
to use a Linux virtual machine to run my network stack. The machine I've used
runs nixOS (via [orbstack](https://orbstack.dev)), and it's configured with
```nix
nix.settings.experimental-features = [ "nix-command" "flakes" ];
programs.git.enable = true;

# Linux and POSIX manual pages (not required for running the project, but useful
# for development)
documentation.dev.enable = true;
environment.systemPackages = with pkgs; [ man-pages man-pages-posix ];

# Setup a tun device so that the network stack can send and receive Ethernet
# frames.
networking.interfaces.tap0.virtual = true;
networking.interfaces.tap0.virtualType = "tap";
networking.interfaces.tap0.virtualOwner = "your-username";
```
The flake is then run with `nix develop`.

## Build

To build the project, run `nix develop` to enter the dev environment,
configure and build the project with CMake, and run the resulting binary:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/netstack
```

## ARP

So far, ARP works (with IPv4 as the protocol). Here is an example of `arping`
(which doesn't update the system's ARP table).

```
[pablo@netstack:~/netstack]$ sudo arping -I tap1 10.0.02
ARPING 10.0.0.2
60 bytes from 00:00:00:00:00:01 (10.0.0.2): index=0 time=650.127 usec
60 bytes from 00:00:00:00:00:01 (10.0.0.2): index=1 time=174.417 usec
^C
--- 10.0.0.2 statistics ---
2 packets transmitted, 2 packets received,   0% unanswered (0 extra)
rtt min/avg/max/std-dev = 0.174/0.412/0.650/0.238 ms

[pablo@netstack:/Users/pablo/Developer/repos/tcpip]$ arp
Address                  HWtype  HWaddress           Flags Mask            Iface
_gateway                 ether   da:9b:d0:54:e0:02   C                     eth0
```

And here one of the system's ping not working (ICMP not yet implemented),
but the device being discovered via ARP, and being in the ARP table.

```
[pablo@netstack:~/netstack]$ ping -c1 -I tap1 10.0.0.2
PING 10.0.0.2 (10.0.0.2) from 10.0.0.1 tap1: 56(84) bytes of data.

--- 10.0.0.2 ping statistics ---
1 packets transmitted, 0 received, 100% packet loss, time 0ms


[pablo@netstack:~/netstack]$ ip neigh show dev tap1
10.0.0.2 lladdr 00:00:00:00:00:01 REACHABLE

[pablo@netstack:~/netstack]$ arp
Address                  HWtype  HWaddress           Flags Mask            Iface
_gateway                 ether   da:9b:d0:54:e0:02   C                     eth0
10.0.0.2                 ether   00:00:00:00:00:01   C                     tap1
```
