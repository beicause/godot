dev:
    scons use_llvm=yes dev_build=yes deprecated=no tests=yes platform=linuxbsd target=editor compiledb=yes

prod:
    scons use_llvm=yes dev_build=no deprecated=no tests=no platform=linuxbsd target=editor compiledb=yes