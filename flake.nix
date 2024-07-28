{
  description = "C++ development environment + Filesystem Hierarchy Standard (FHS)";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { nixpkgs, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      # buildFHSEnv -> https://nixos.org/manual/nixpkgs/stable/#sec-fhs-environments
      # The packages included in appimagetools.defaultFhsEnvArgs are:
      # https://github.com/NixOS/nixpkgs/blob/nixos-unstable/pkgs/build-support/appimage/default.nix#L72-L208

      devShells.${system}.default =
        (pkgs.buildFHSEnv (
          pkgs.appimageTools.defaultFhsEnvArgs
          // {
            name = "dev";
            # Packages installed in the development shell
            targetPkgs =
              pkgs: with pkgs; [
                pkg-config
                autoPatchelfHook
                clang_18
                clang-tools
                python3
                scons
                cxx-rs
                pre-commit
              ];
            # Commands to be executed in the development shell
            profile = ''
              echo ------Godot C++ Scons Development Environment-------
              IN_NIX_SHELL=1 NIX_SHELL_NAME=gd-env exec fish
            '';
          }
        )).env;
    };
}
