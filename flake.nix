{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { nixpkgs, ... }:
    let
      systems = [ "aarch64-linux" "x86_64-linux" ];
      forAll = f: nixpkgs.lib.genAttrs systems (s: f nixpkgs.legacyPackages.${s});
    in {
      devShells = forAll (pkgs: {
        default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }){
          packages = with pkgs; [
            cmake ninja gdb clang-tools
            tcpdump iproute2 netcat-openbsd curl
          ];
        };
      });
    };
}
