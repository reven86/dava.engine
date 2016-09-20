import sys
import os
import shutil
import subprocess
import build_utils
import build_config

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return ['win32', 'win10']
	else:
		return []

def get_dependencies_for_target(target):
	return []

def get_supported_build_platforms():
	# Shouldn't be built on macOS since it's already bundled in macOS sdk, iOS sdk, Android sdk
	return ['win32']

def build_for_target(target, working_directory_path, root_project_path):
	if target == 'all':
		__build_all_on_current_platform(working_directory_path, root_project_path)
	elif target == 'win32':
		__build_win32(working_directory_path, root_project_path)
	elif target == 'win10':
		__build_win10(working_directory_path, root_project_path)
	else:
		return

def get_download_url():
	return 'http://zlib.net/zlib128.zip'

def __get_downloaded_archive_inner_dir():
	return 'zlib-1.2.8'

def __download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'zlib_source')
	build_utils.download_and_extract(get_download_url(), working_directory_path, source_folder_path, __get_downloaded_archive_inner_dir())
	return source_folder_path

def __build_all_on_current_platform(working_directory_path, root_project_path):
	if sys.platform == 'win32':
		return (__build_win32(working_directory_path, root_project_path) and
				__build_win10(working_directory_path, root_project_path))

def __build_win32(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)

	# Folders for the library to be built into
	build_x86_folder = os.path.join(working_directory_path, 'gen/build_win32_x86')
	build_x64_folder = os.path.join(working_directory_path, 'gen/build_win32_x64')

	# Build params
	solution_name = 'zlib.sln'
	target_name = 'zlibstatic'

	# Generate & build
	build_utils.cmake_generate_build_vs(build_x86_folder, source_folder_path, build_config.win32_x86_cmake_generator, solution_name, target_name, 'Win32')
	build_utils.cmake_generate_build_vs(build_x64_folder, source_folder_path, build_config.win32_x64_cmake_generator, solution_name, target_name, 'Win64')
	
	# Copy .lib files
	# TODO: update pathes after switching to new folders structure
	# TODO: change z.lib to zlib.lib for win-x64
	lib_path_x86_debug = os.path.join(build_x86_folder, 'Debug/zlibstaticd.lib')
	lib_path_x86_release = os.path.join(build_x86_folder, 'Release/zlibstatic.lib')
	lib_path_x64_debug = os.path.join(build_x64_folder, 'Debug/zlibstaticd.lib')
	lib_path_x64_release = os.path.join(build_x64_folder, 'Release/zlibstatic.lib')

	shutil.copyfile(lib_path_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Debug/zlib.lib'))
	shutil.copyfile(lib_path_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Release/zlib.lib'))
	shutil.copyfile(lib_path_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Debug/z.lib'))
	shutil.copyfile(lib_path_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Release/z.lib'))

	# Copy created configuration header to root folder
	# Required to use source folder as include path
	# TODO: get rid of this and copy to Libs/Include directly
	shutil.copyfile(os.path.join(build_x86_folder, 'zconf.h'), os.path.join(source_folder_path, 'zconf.h'))

	return True
	
def __build_win10(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	
	# Folders for the library to be built into
	build_win10_x86_folder = os.path.join(working_directory_path, 'gen/build_win10_x86')
	build_win10_x64_folder = os.path.join(working_directory_path, 'gen/build_win10_x64')
	build_win10_arm_folder = os.path.join(working_directory_path, 'gen/build_win10_arm')

	# Build params
	solution_name = 'zlib.sln'
	target_name = 'zlibstatic'
	cmake_win10_flags = ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0']

	# Generate & build
	build_utils.cmake_generate_build_vs(build_win10_x86_folder, source_folder_path, 'Visual Studio 14 2015', solution_name, target_name, 'Win32', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_x64_folder, source_folder_path, 'Visual Studio 14 2015 Win64', solution_name, target_name, 'Win64', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_arm_folder, source_folder_path, 'Visual Studio 14 2015 ARM', solution_name, target_name, 'ARM', cmake_win10_flags)

	lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, 'Debug/zlibstaticd.lib')
	lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, 'Release/zlibstatic.lib')
	lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, 'Debug/zlibstaticd.lib')
	lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, 'Release/zlibstatic.lib')
	lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, 'Debug/zlibstaticd.lib')
	lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, 'Release/zlibstatic.lib')

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure	
	shutil.copyfile(lib_path_win10_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Debug/zlib.lib'))
	shutil.copyfile(lib_path_win10_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Release/zlib.lib'))
	shutil.copyfile(lib_path_win10_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Debug/zlib.lib'))
	shutil.copyfile(lib_path_win10_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Release/zlib.lib'))
	shutil.copyfile(lib_path_win10_arm_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Debug/zlib.lib'))
	shutil.copyfile(lib_path_win10_arm_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Release/zlib.lib'))

	# Copy created configuration header to root folder
	# Required to use source folder as include path
	# TODO: get rid of this and copy to Libs/Include directly
	shutil.copyfile(os.path.join(build_win10_x86_folder, 'zconf.h'), os.path.join(source_folder_path, 'zconf.h'))

	return True