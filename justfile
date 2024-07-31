dev:
    scons use_llvm=yes dev_build=yes platform=linuxbsd target=editor compiledb=yes

prod:
    scons use_llvm=yes dev_build=no platform=linuxbsd target=editor compiledb=yes
