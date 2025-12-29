{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, ... }:
  let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;

    simapi = pkgs.stdenv.mkDerivation {
      name = "simapi";
      version = "0.1.0";

      src = ./.;

      nativeBuildInputs = [
        pkgs.cmake
      ];

      cmakeFlags = [
        "-DBUILD_SIMD=OFF"
      ];
    };

    simd = pkgs.stdenv.mkDerivation {
      name = "simd";
      version = "0.1.0";

      src = ./.;

      preConfigure = ''
        cd simd
      '';

      nativeBuildInputs = [
        pkgs.cmake
      ];

      buildInputs = [
        simapi

        pkgs.libuv
        pkgs.yder
        pkgs.libconfig
        pkgs.argtable
      ];

      installPhase = ''
        install -D -m 755 -v ./simd $out/bin/simd
      '';
    };
  in
  {
    packages.x86_64-linux = {
      default = simapi;
      inherit simapi simd;
    };

    devShells.x86_64-linux.default = pkgs.mkShell {
      packages = simapi.buildInputs ++ simd.buildInputs ++ [
        simapi
      ];
    };
  };
}
