import sys
import os
import shutil
import subprocess
import build_utils

def get_internal_dependencies_for_platform(platform):
	return []

def get_supported_build_platforms():
	# Shouldn't be built on macOS since it's already bundled in macOS sdk, iOS sdk, Android sdk
	return ['win32']

def build(output_folder_path, root_project_path):
	zlib_tmp_source_filepath = os.path.join(output_folder_path, 'zlib_source.zip')
	zlib_source_folder_path = os.path.join(output_folder_path, 'zlib_source')

	build_utils.download_if_doesnt_exist('http://zlib.net/zlib128.zip', zlib_tmp_source_filepath)
	build_utils.unzip_inplace(zlib_tmp_source_filepath)
	shutil.move(os.path.join(output_folder_path, 'zlib-1.2.8'), zlib_source_folder_path)
	
	build_x86_folder = os.path.join(output_folder_path, 'gen/build_x86')
	build_x64_folder = os.path.join(output_folder_path, 'gen/build_x64')
	build_win10_x86_folder = os.path.join(output_folder_path, 'gen/build_win10_x86')
	build_win10_x64_folder = os.path.join(output_folder_path, 'gen/build_win10_x64')
	build_win10_arm_folder = os.path.join(output_folder_path, 'gen/build_win10_arm')

	solution_name = 'zlib.sln'
	target_name = 'zlibstatic'
	cmake_src_dir = os.path.abspath(zlib_source_folder_path)
	cmake_win10_flags = ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0']

	build_utils.cmake_generate_build_vs(build_x86_folder, cmake_src_dir, 'Visual Studio 12', solution_name, target_name, 'Win32')
	build_utils.cmake_generate_build_vs(build_x64_folder, cmake_src_dir, 'Visual Studio 12 Win64', solution_name, target_name, 'Win64')
	build_utils.cmake_generate_build_vs(build_win10_x86_folder, cmake_src_dir, 'Visual Studio 14 2015', solution_name, target_name, 'Win32', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_x64_folder, cmake_src_dir, 'Visual Studio 14 2015 Win64', solution_name, target_name, 'Win64', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_arm_folder, cmake_src_dir, 'Visual Studio 14 2015 ARM', solution_name, target_name, 'ARM', cmake_win10_flags)

	# Copy created configuration header to root folder
	shutil.copyfile(os.path.join(build_x86_folder, 'zconf.h'), os.path.join(zlib_source_folder_path, 'zconf.h'))

	# Rename libraries to zlib.lib

	lib_path_x86_debug = os.path.join(build_x86_folder, 'Debug/zlib.lib')
	lib_path_x86_release = os.path.join(build_x86_folder, 'Release/zlib.lib')
	lib_path_x64_debug = os.path.join(build_x64_folder, 'Debug/zlib.lib')
	lib_path_x64_release = os.path.join(build_x64_folder, 'Release/zlib.lib')
	lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, 'Debug/zlib.lib')
	lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, 'Release/zlib.lib')
	lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, 'Debug/zlib.lib')
	lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, 'Release/zlib.lib')
	lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, 'Debug/zlib.lib')
	lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, 'Release/zlib.lib')

	shutil.move(os.path.join(build_x86_folder, 'Debug/zlibstaticd.lib'), lib_path_x86_debug)
	shutil.move(os.path.join(build_x86_folder, 'Release/zlibstatic.lib'), lib_path_x86_release)
	shutil.move(os.path.join(build_x64_folder, 'Debug/zlibstaticd.lib'), lib_path_x64_debug)
	shutil.move(os.path.join(build_x64_folder, 'Release/zlibstatic.lib'), lib_path_x64_release)
	shutil.move(os.path.join(build_win10_x86_folder, 'Debug/zlibstaticd.lib'), lib_path_win10_x86_debug)
	shutil.move(os.path.join(build_win10_x86_folder, 'Release/zlibstatic.lib'), lib_path_win10_x86_release)
	shutil.move(os.path.join(build_win10_x64_folder, 'Debug/zlibstaticd.lib'), lib_path_win10_x64_debug)
	shutil.move(os.path.join(build_win10_x64_folder, 'Release/zlibstatic.lib'), lib_path_win10_x64_release)
	shutil.move(os.path.join(build_win10_arm_folder, 'Debug/zlibstaticd.lib'), lib_path_win10_arm_debug)
	shutil.move(os.path.join(build_win10_arm_folder, 'Release/zlibstatic.lib'), lib_path_win10_arm_release)

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure
	# TODO: change z.lib to zlib.lib for win-x64

	shutil.copyfile(lib_path_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Debug/zlib.lib'))
	shutil.copyfile(lib_path_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Release/zlib.lib'))
	shutil.copyfile(lib_path_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Debug/z.lib'))
	shutil.copyfile(lib_path_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Release/z.lib'))
	shutil.copyfile(lib_path_win10_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Debug/zlib.lib'))
	shutil.copyfile(lib_path_win10_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Release/zlib.lib'))
	shutil.copyfile(lib_path_win10_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Debug/zlib.lib'))
	shutil.copyfile(lib_path_win10_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Release/zlib.lib'))
	shutil.copyfile(lib_path_win10_arm_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Debug/zlib.lib'))
	shutil.copyfile(lib_path_win10_arm_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Release/zlib.lib'))

	return True