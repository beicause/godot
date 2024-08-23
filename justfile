set shell := ["fish", "-c"]

c := ""

base :="\
	-j14 \
	c_compiler_launcher=ccache cpp_compiler_launcher=ccache \
	scu_build=yes \
	warnings=extra \
	werror=yes \
	strict_checks=yes \
	fast_unsafe=yes \
\
	disable_xr=yes \
	accesskit=no \
	opengl3=no \
	engine_update_check=no \
\
	module_mono_enabled=yes \
	module_camera_enabled=no \
	module_webxr_enabled=no \
	module_webrtc_enabled=no \
	module_openxr_enabled=no \
	module_mobile_vr_enabled=no \
	module_godot_physics_3d_enabled=no \
	module_csg_enabled=no \
	module_text_server_fb_enabled=yes \
	module_text_server_adv_enabled=no \
	module_fbx_enabled=no \
\
    voxel_tests=no \
    voxel_smooth_meshing=no \
    voxel_modifiers=no \
    voxel_sqlite=no \
    voxel_instancer=no \
    voxel_gpu=no \
    voxel_basic_generators=no \
    voxel_mesh_sdf=no \
    voxel_vox=no \
	tracy=no \
	voxel_fast_noise_2=yes \
	" + c

dev_base := "\
	linker=mold \
	use_llvm=yes \
	dev_build=yes \
	platform=linuxbsd \
	target=editor " + base


prod_base :=  " deprecated=no " + base

template_base := " disable_physics_2d=yes " + prod_base

default: editor

dev:
	scons compiledb=yes {{dev_base}}

dev-f:
	scons compiledb=no {{dev_base}}

dev-test:
	scons tests=yes compiledb=yes {{dev_base}}

dev-asan:
	scons use_asan=yes {{dev_base}}

editor:
	scons platform=linuxbsd target=editor debug_symbols=yes compiledb=no use_llvm=yes linker=mold \
	{{prod_base}}

android_debug:
	scons dev_build=no platform=android target=template_debug {{template_base}}

android_release:
	scons dev_build=no platform=android target=template_release {{template_base}}

linux_debug:
	scons dev_build=no use_llvm=yes linker=mold platform=linuxbsd target=template_debug {{template_base}}

linux_release:
	scons dev_build=no use_llvm=yes linker=mold platform=linuxbsd target=template_release {{template_base}}

windows_debug:
	scons dev_build=yes platform=windows target=template_debug {{template_base}}

windows_debug_mingw_llvm:
	scons dev_build=yes platform=windows target=template_debug use_llvm=yes use_mingw=yes {{template_base}}

windows_debug_mingw_gcc:
	scons dev_build=yes platform=windows target=template_debug use_llvm=no use_mingw=yes {{template_base}}

web_debug:
	scons dev_build=no platform=web target=template_debug {{template_base}} module_mono_enabled=no

mono_glue:
	./bin/godot.linuxbsd.editor.x86_64.llvm.mono --headless --generate-mono-glue modules/mono/glue

mono_sdk:
	./modules/mono/build_scripts/build_assemblies.py --godot-output-dir ./bin --push-nupkgs-local \
	~/MyLocalNugetSource/ --no-deprecated
