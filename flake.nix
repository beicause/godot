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
      deps = with pkgs; [
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
      shell_name = "gd-env";
      extra_envs = {
        IN_NIX_SHELL = "1";
        ANDROID_NDK_HOME = "~/Android/ndk/23.2.8568313/";
        NIX_SHELL_NAME = shell_name;
      };
      fhs_env =
        script:
        (pkgs.buildFHSUserEnv (
          pkgs.appimageTools.defaultFhsEnvArgs
          // {
            name = "fhs-env";
            targetPkgs = pkgs: deps;
            profile = ''
              export IN_NIX_SHELL=${extra_envs.IN_NIX_SHELL};
              export ANDROID_NDK_HOME="${extra_envs.ANDROID_NDK_HOME}";
              export NIX_SHELL_NAME="${extra_envs.NIX_SHELL_NAME}";
            '';
            runScript = script;
          }
        ));
      # build_dev_script = "scons use_llvm=yes dev_build=yes deprecated=no tests=no platform=linuxbsd target=editor compiledb=yes";
      # build_prod_script = "scons use_llvm=yes dev_build=no deprecated=no tests=no platform=linuxbsd target=editor compiledb=yes";
    in
    {
      devShells."${system}" = {
        default = (fhs_env "fish").env;
        build_dev = (fhs_env "just dev").env;
        build_prod = (fhs_env "just prod").env;
        run_bin = (fhs_env "./bin/godot.linuxbsd.editor.dev.x86_64.llvm --editor --path ~/game").env;
      };
    };
}
