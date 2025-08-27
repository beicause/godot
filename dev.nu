#!/usr/bin/env nu

use std/dirs

export-env {
	use std/dirs
}

const current_dir = path self .

const base = "
	-j15
	c_compiler_launcher=ccache cpp_compiler_launcher=ccache
	scu_build=yes
	warnings=extra
	werror=yes
	strict_checks=yes
	fast_unsafe=yes

	disable_xr=yes
	accesskit=yes
	engine_update_check=no

	module_mono_enabled=no
	module_camera_enabled=no
	module_webxr_enabled=no
	module_webrtc_enabled=no
	module_openxr_enabled=no
	module_mobile_vr_enabled=no
	module_godot_physics_3d_enabled=no
	module_csg_enabled=no
	module_text_server_fb_enabled=yes
	module_text_server_adv_enabled=no
	module_fbx_enabled=no

	custom_modules=../godot-modules
	voxel_tests=no
	voxel_smooth_meshing=no
	voxel_modifiers=no
	voxel_sqlite=yes
	voxel_instancer=no
	voxel_gpu=no
	voxel_basic_generators=no
	voxel_mesh_sdf=no
	voxel_vox=no
	voxel_fast_noise_2=no
	voxel_werror=yes
"

const dev_base = "
	linker=mold
	use_llvm=yes
	dev_build=yes
	platform=linuxbsd
	target=editor
	" + $base

const template_base = "
	visual_shader=no
	opengl3=no
	disable_physics_2d=yes
	" + $base

def run_cmd [cmd:string] {
	let cmd = $cmd | split row -r '\s+' | where ($it | is-not-empty);
	print ($cmd | str join " ");
	run-external $cmd
}

export def main [-c:string] {
	dev editor_linux_debug -c ($c | default "")
}

export def "dev editor_linux_opt" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons platform=linuxbsd target=editor debug_symbols=yes use_llvm=yes linker=mold ($base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev editor_linux_debug" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons ($dev_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev editor_linux_debug_test" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons tests=yes ($dev_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev editor_linux_debug_asan" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons use_asan=yes use_lsan=yes use_ubsan=yes ($dev_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev editor_linux_debug_tsan" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons use_tsan=yes ($dev_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev editor_linux_debug_msan" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons use_msan=yes ($dev_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_android_debug" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=no platform=android target=template_debug ($template_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_android_release" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=no platform=android target=template_release ($template_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_linux_debug" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=no use_llvm=yes linker=mold platform=linuxbsd target=template_deug ($template_base) ($c))";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_linux_release" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=no use_llvm=yes linker=mold platform=linuxbsd target=template_relese ($template_base) ($c))";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_windows_debug" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=yes platform=windows target=template_debug ($template_base) ($c)";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_windows_debug_mingw_llvm" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=yes platform=windows target=template_debug use_llvm=yes use_mingw=es ($template_base) ($c))";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_windows_debug_mingw_gcc" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=yes platform=windows target=template_debug use_llvm=no use_mingw=es ($template_base) ($c))";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev template_web_debug" [-c:string] {
	dirs add $current_dir
	(
		let cmd = $"scons dev_build=no platform=web target=template_deug ($template_base) module_mono_enabled=no ($c))";
		run_cmd $cmd
	)
	dirs drop
}

export def "dev mono_gen_glue" [] {
	dirs add $current_dir
	(
		./bin/godot.linuxbsd.editor.x86_64.llvm.mono --headless --generate-mono-glue modules/mono/glue
	)
	dirs drop
}

export def "dev mono_build_sdk" [] {
	dirs add $current_dir
	(
		./modules/mono/build_scripts/build_assemblies.py --godot-output-dir ./bin --push-nupkgs-local ~/MyLocalNugetSource/ --no-deprecated
	)
	dirs drop
}
