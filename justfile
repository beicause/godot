set shell := ["fish", "-c"]

base :="scu_build=yes warnings=extra werror=yes"
dev_base := "scons linker=mold use_llvm=yes dev_build=yes platform=linuxbsd target=editor " + base

dev:
    {{dev_base}} compiledb=yes

dev-test:
    {{dev_base}} tests=yes compiledb=yes

dev-asan:
    {{dev_base}} use_asan=yes

prod:
    scons platform=linuxbsd target=editor compiledb=yes use_llvm=yes linker=mold {{base}} \
    deprecated="no" \
    module_mobile_vr_enabled="no" \
    module_openxr_enabled="no" \
    module_webrtc_enabled="no" \
    module_webxr_enabled="no" \

template-debug-android:
    scons dev_build=yes platform=android target=template_debug {{base}} \
    disable_3d="yes" \
    deprecated="no" \
    module_mobile_vr_enabled="no" \
    module_openxr_enabled="no" \
    module_webrtc_enabled="no" \
    module_webxr_enabled="no" \
