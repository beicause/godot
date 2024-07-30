{
  description = "Godot development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };
    in
    {
      devShells."${system}".default =
        (pkgs.buildFHSUserEnv (
          pkgs.appimageTools.defaultFhsEnvArgs
          // {
            name = "fhs-env";
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
                # Install Android SDK:
                # sdkmanager --sdk_root=<android_sdk_path> --licenses
                # sdkmanager --sdk_root=<android_sdk_path> "platform-tools" "build-tools;34.0.0" "platforms;android-34" "cmdline-tools;latest" "cmake;3.10.2.4988404" "ndk;23.2.8568313"
              ];
            profile = ''
              echo ------Godot C++ Scons Development Environment-------
              export IN_NIX_SHELL=1;
              export ANDROID_NDK_HOME="~/Android/ndk/23.2.8568313/";
              export NIX_SHELL_NAME="gd-env";
            '';
            runScript = "fish";
          }
        )).env;
    };
}
