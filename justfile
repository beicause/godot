set shell := ["fish", "-c"]

c := ""

base :="\
	cache_path=bin/scons_cache cache_limit=4 \
	scu_build=yes \
	warnings=extra \
	werror=yes \
	module_mono_enabled=yes disable_xr=yes strict_checks=yes " + c

dev_base := "\
	linker=mold \
	use_llvm=yes \
	dev_build=yes \
	platform=linuxbsd \
	target=editor " + base


prod_base :=  "\
	deprecated=no \
	module_godot_physics_3d_enabled=no \
	module_mobile_vr_enabled=no \
	module_csg_enabled=no \
	module_openxr_enabled=no \
	module_webrtc_enabled=no \
	module_webxr_enabled=no \
	module_text_server_fb_enabled=yes \
	module_text_server_adv_enabled=no \
	module_multiplayer_enabled=no \
	module_enet_enabled=no \
	module_fbx_enabled=no \
	module_ktx_enabled=no \
	module_jsonrpc_enabled=no " + base

template_base := prod_base

default: editor

dev:
	scons compiledb=yes {{dev_base}}

dev-test:
	scons tests=yes compiledb=yes {{dev_base}}

dev-asan:
	scons use_asan=yes {{dev_base}}

editor:
	scons platform=linuxbsd target=editor debug_symbols=yes compiledb=no use_llvm=yes linker=mold \
	{{prod_base}} module_jsonrpc_enabled=yes module_multiplayer_enabled=yes module_enet_enabled=yes

android_debug:
	scons dev_build=no platform=android target=template_debug {{template_base}}

android_release:
	scons dev_build=no platform=android target=template_release {{template_base}}

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
