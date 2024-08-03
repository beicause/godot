set shell := ["fish", "-c"]

dev_base := "scons use_llvm=no linker=lld scu_build=yes dev_build=yes platform=linuxbsd target=editor"

dev:
    {{dev_base}} compiledb=yes

dev-test:
    {{dev_base}} tests=yes

dev-asan:
    {{dev_base}} use_asan=yes

dev-min:
    scons use_llvm=no dev_build=yes linker=lld platform=linuxbsd target=template_debug use_asan=yes modules_enabled_by_default=no disable_3d=yes deprecated=no disable_advanced_gui=yes

prod:
    scons use_llvm=no dev_build=no platform=linuxbsd target=editor compiledb=yes scu_build=yes linker=lld \
    deprecated="no" \
    module_mobile_vr_enabled="no" \
    module_openxr_enabled="no" \
    module_webrtc_enabled="no" \
    module_webxr_enabled="no" \

template-debug-android:
    scons dev_build=yes platform=android target=template_debug compiledb=yes \
    disable_3d="yes" \
    deprecated="no" \
    module_mobile_vr_enabled="no" \
    module_openxr_enabled="no" \
    module_webrtc_enabled="no" \
    module_webxr_enabled="no" \
