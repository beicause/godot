{
  description = "C++ development environment + Filesystem Hierarchy Standard (FHS)";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { nixpkgs, ... }@inputs:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };
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
                nodejs_22
                rustup
                cargo-ndk
                jdk17
                android-studio-tools
              ];
            profile = ''
              echo ------Godot C++ Scons Development Environment-------
              IN_NIX_SHELL=1 NIX_SHELL_NAME=gd-env ANDROID_NDK_HOME=~/Android/ndk/23.2.8568313/ exec fish
            '';
            # Install Android SDK:
            # sdkmanager --sdk_root=<android_sdk_path> --licenses
            # sdkmanager --sdk_root=<android_sdk_path> "platform-tools" "build-tools;34.0.0" "platforms;android-34" "cmdline-tools;latest" "cmake;3.10.2.4988404" "ndk;23.2.8568313"
          }
        )).env;
    };
}
