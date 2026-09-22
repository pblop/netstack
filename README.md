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

