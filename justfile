dev:
    scons use_llvm=yes dev_build=yes platform=linuxbsd target=editor compiledb=yes

prod:
    scons use_llvm=yes dev_build=no platform=linuxbsd target=editor compiledb=yes

template_debug_android:
    scons use_llvm=yes dev_build=yes platform=android target=template_debug compiledb=yes \
    disable_3d="yes" \
    deprecated="no" \
    module_mobile_vr_enabled="no" \
    module_openxr_enabled="no" \
    module_webrtc_enabled="no" \
    module_webxr_enabled="no" \
