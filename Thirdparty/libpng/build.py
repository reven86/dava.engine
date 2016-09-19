import sys
sys.path.append('../Private')

import os
import shutil
import subprocess
import build_utils

# Download & extract

libpng_sources_filename = 'tmp/libpng_source.zip'
build_utils.download_if_doesnt_exist('ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/lpng1625.zip', libpng_sources_filename)
build_utils.unzip_inplace(libpng_sources_filename)
shutil.move('tmp/lpng1625', 'tmp/libpng_source')

build_utils.apply_patch('patch.diff')

# Add configuration file
shutil.copyfile('pngusr.dfa', os.path.join('tmp/libpng_source', 'pngusr.dfa'))

if sys.platform == "win32":
	# Build

	build_x86_folder = 'tmp/gen/build_x86'
	build_x64_folder = 'tmp/gen/build_x64'
	build_win10_x86_folder = 'tmp/gen/build_win10_x86'
	build_win10_x64_folder = 'tmp/gen/build_win10_x64'
	build_win10_arm_folder = 'tmp/gen/build_win10_arm'
	build_android_folder = 'tmp/gen/build_android'

	solution_name = 'libpng.sln'
	target_name = 'png_static'
	cmake_src_dir = '../../libpng_source'
	cmake_flags =  [ '-DZLIB_LIBRARY=../../../../zlib/tmp/gen/build_x86/Release/zlib.lib', '-DZLIB_INCLUDE_DIR=../../../../zlib/tmp/zlib_source/' ]
	cmake_win10_flags = cmake_flags + ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0' ]

	build_utils.cmake_generate_build_vs(build_x86_folder, cmake_src_dir, 'Visual Studio 12', solution_name, target_name, 'Win32', cmake_flags)
	build_utils.cmake_generate_build_vs(build_x64_folder, cmake_src_dir, 'Visual Studio 12 Win64', solution_name, target_name, 'Win64', cmake_flags)
	build_utils.cmake_generate_build_vs(build_win10_x86_folder, cmake_src_dir, 'Visual Studio 14 2015', solution_name, target_name, 'Win32', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_x64_folder, cmake_src_dir, 'Visual Studio 14 2015 Win64', solution_name, target_name, 'Win64', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_arm_folder, cmake_src_dir, 'Visual Studio 14 2015 ARM', solution_name, target_name, 'ARM', cmake_win10_flags)

	# Copy created configuration header to root folder
	shutil.copyfile(os.path.join(build_x86_folder, 'pnglibconf.h'), 'tmp/libpng_source/pnglibconf.h')

	build_utils.build_android_ndk('android_ndk_project', os.path.join('..', build_android_folder), debug=False)

	# Rename libraries to libpng.lib

	lib_path_x86_debug = os.path.join(build_x86_folder, 'Debug/libpng.lib')
	lib_path_x86_release = os.path.join(build_x86_folder, 'Release/libpng.lib')
	lib_path_x64_debug = os.path.join(build_x64_folder, 'Debug/libpng.lib')
	lib_path_x64_release = os.path.join(build_x64_folder, 'Release/libpng.lib')
	lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, 'Debug/libpng.lib')
	lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, 'Release/libpng.lib')
	lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, 'Debug/libpng.lib')
	lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, 'Release/libpng.lib')
	lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, 'Debug/libpng.lib')
	lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, 'Release/libpng.lib')
	lib_path_android_armeabiv7a = os.path.join(build_android_folder, 'local/armeabi-v7a/libpng.a')
	lib_path_android_x86 = os.path.join(build_android_folder, 'local/x86/libpng.a')

	shutil.move(os.path.join(build_x86_folder, 'Debug/libpng16_staticd.lib'), lib_path_x86_debug)
	shutil.move(os.path.join(build_x86_folder, 'Release/libpng16_static.lib'), lib_path_x86_release)
	shutil.move(os.path.join(build_x64_folder, 'Debug/libpng16_staticd.lib'), lib_path_x64_debug)
	shutil.move(os.path.join(build_x64_folder, 'Release/libpng16_static.lib'), lib_path_x64_release)
	shutil.move(os.path.join(build_win10_x86_folder, 'Debug/libpng16_staticd.lib'), lib_path_win10_x86_debug)
	shutil.move(os.path.join(build_win10_x86_folder, 'Release/libpng16_static.lib'), lib_path_win10_x86_release)
	shutil.move(os.path.join(build_win10_x64_folder, 'Debug/libpng16_staticd.lib'), lib_path_win10_x64_debug)
	shutil.move(os.path.join(build_win10_x64_folder, 'Release/libpng16_static.lib'), lib_path_win10_x64_release)
	shutil.move(os.path.join(build_win10_arm_folder, 'Debug/libpng16_staticd.lib'), lib_path_win10_arm_debug)
	shutil.move(os.path.join(build_win10_arm_folder, 'Release/libpng16_static.lib'), lib_path_win10_arm_release)

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	shutil.copyfile(lib_path_x86_debug, '../../Libs/lib_CMake/win/x86/Debug/pnglib_wind.lib')
	shutil.copyfile(lib_path_x86_release, '../../Libs/lib_CMake/win/x86/Release/pnglib_win.lib')
	shutil.copyfile(lib_path_x64_debug, '../../Libs/lib_CMake/win/x64/Debug/pnglib_wind.lib')
	shutil.copyfile(lib_path_x64_release, '../../Libs/lib_CMake/win/x64/Release/pnglib_win.lib')
	shutil.copyfile(lib_path_win10_x86_debug, '../../Libs/lib_CMake/win10/Win32/Debug/pnglib_wind.lib')
	shutil.copyfile(lib_path_win10_x86_release, '../../Libs/lib_CMake/win10/Win32/Release/pnglib_win.lib')
	shutil.copyfile(lib_path_win10_x64_debug, '../../Libs/lib_CMake/win10/x64/Debug/pnglib_wind.lib')
	shutil.copyfile(lib_path_win10_x64_release, '../../Libs/lib_CMake/win10/x64/Release/pnglib_win.lib')
	shutil.copyfile(lib_path_win10_arm_debug, '../../Libs/lib_CMake/win10/arm/Debug/pnglib_wind.lib')
	shutil.copyfile(lib_path_win10_arm_release, '../../Libs/lib_CMake/win10/arm/Release/pnglib_win.lib')
	shutil.copyfile(lib_path_android_armeabiv7a, '../../Libs/lib_CMake/android/armeabi-v7a/libpng.a')
	shutil.copyfile(lib_path_android_x86, '../../Libs/lib_CMake/android/x86/libpng.a')