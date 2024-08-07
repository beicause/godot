set shell := ["fish", "-c"]

c := ""
base :="scu_build=yes warnings=extra werror=yes " + c
dev_base := "linker=mold use_llvm=yes dev_build=yes platform=linuxbsd target=editor " + base
template :=  "deprecated=no module_mobile_vr_enabled=no module_csg_enabled=no module_openxr_enabled=no module_webrtc_enabled=no module_webxr_enabled=no " + base

dev:
    scons compiledb=yes {{dev_base}}

dev-test:
    scons tests=yes compiledb=yes {{dev_base}}

dev-asan:
    scons use_asan=yes {{dev_base}}

editor:
    scons platform=linuxbsd target=editor lto=none compiledb=no use_llvm=yes linker=mold \
    deprecated=no \
    module_mobile_vr_enabled=no \
    module_openxr_enabled=no \
    module_webrtc_enabled=no \
    module_csg_enabled=no \
    module_webxr_enabled=no {{base}}

android_debug:
    scons dev_build=yes platform=android target=template_debug {{template}}

windows_debug:
    scons dev_build=yes platform=windows target=template_debug {{template}}

windows_debug_mingw:
    scons dev_build=yes platform=windows target=template_debug use_llvm=yes use_mingw=yes {{template}}
