import sys
import os
import shutil
import subprocess
import build_utils
import build_config

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return ['win32', 'win10', 'android']
	else:
		return ['macos', 'ios', 'android']

def get_dependencies_for_target(target):
	if target in ['win32', 'win10']:
		return ['zlib']
	else:
		return []

def get_supported_build_platforms():
	return ['win32', 'darwin']

def build_for_target(target, working_directory_path, root_project_path):
	if target == 'all':
		__build_all_on_current_platform(working_directory_path, root_project_path)
	elif target == 'win32':
		__build_win32(working_directory_path, root_project_path)
	elif target == 'win10':
		__build_win10(working_directory_path, root_project_path)
	elif target == 'macos':
		__build_macos(working_directory_path, root_project_path)
	elif target == 'ios':
		__build_ios(working_directory_path, root_project_path)
	elif target == 'android':
		__build_android(working_directory_path, root_project_path)

def get_download_url():
	return {'win32' : 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/lpng1625.zip', 'others' : 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.25.tar.gz'}

def __get_download_archive_inner_dir():
	if sys.platform == 'win32':
		return 'lpng1625'
	else:
		return 'libpng-1.6.25'

def __download_and_extract(working_directory_path):
	# Path to extracted source folder
	source_folder_path = os.path.join(working_directory_path, 'libpng_source')

	# Skip if we've already did the job once
	try:
		if __download_and_extract.did:
			return source_folder_path
	except AttributeError:
		pass

	# Download otherwise

	if sys.platform == 'win32':
		download_link = get_download_url()['win32']
	else:
		download_link = get_download_url()['others']

	sources_filename = download_link.split('/')[-1]

	# Path to downloaded archive
	source_archive_filepath = os.path.join(working_directory_path, sources_filename)

	# Download & unarchive
	build_utils.download_if_doesnt_exist(download_link, source_archive_filepath)
	build_utils.unzip_inplace(source_archive_filepath)

	# Rename version-dependent folder name to simpler one
	# In case other builder will need to use this folder
	shutil.move(os.path.join(working_directory_path, __get_download_archive_inner_dir()), source_folder_path)

	__download_and_extract.did = True

	return source_folder_path

def __patch_sources(source_folder_path, working_directory_path):
	# Skip if we've already did the job once
	try:
		if __patch_sources.did:
			return
	except AttributeError:
		pass

	# Apply fixes
	build_utils.apply_patch(os.path.abspath('patch.diff'), working_directory_path)

	# Add configuration file to source folder
	# It is used to generate additional header & source files
	shutil.copyfile('pngusr.dfa', os.path.join(source_folder_path, 'pngusr.dfa'))

	__patch_sources.did = True

def __build_all_on_current_platform(working_directory_path, root_project_path):
	if sys.platform == 'win32':
		__build_win32(working_directory_path, root_project_path)
		__build_win10(working_directory_path, root_project_path)
		__build_android(working_directory_path, root_project_path)
	else:
		__build_macos(working_directory_path, root_project_path)
		__build_ios(working_directory_path, root_project_path)
		__build_android(working_directory_path, root_project_path)

def __build_win32(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_x86_folder = os.path.join(working_directory_path, 'gen/build_win32_x86')
	build_x64_folder = os.path.join(working_directory_path, 'gen/build_win32_x64')

	# Build params
	# TODO: Do not use temporary zlib output, switch to repo/Libs/ instead
	solution_name = 'libpng.sln'
	target_name = 'png_static'
	cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(working_directory_path, '../zlib/gen/build_win32_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(working_directory_path, '../zlib/zlib_source/') ]

	# Build all VS projects
	build_utils.cmake_generate_build_vs(build_x86_folder, source_folder_path, build_config.win32_x86_cmake_generator, solution_name, target_name, 'Win32', cmake_flags)
	build_utils.cmake_generate_build_vs(build_x64_folder, source_folder_path, build_config.win32_x64_cmake_generator, solution_name, target_name, 'Win64', cmake_flags)

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_x86_debug = os.path.join(build_x86_folder, 'Debug/libpng16_staticd.lib')
	lib_path_x86_release = os.path.join(build_x86_folder, 'Release/libpng16_static.lib')
	lib_path_x64_debug = os.path.join(build_x64_folder, 'Debug/libpng16_staticd.lib')
	lib_path_x64_release = os.path.join(build_x64_folder, 'Release/libpng16_static.lib')

	shutil.copyfile(lib_path_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Debug/pnglib_wind.lib'))
	shutil.copyfile(lib_path_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Release/pnglib_win.lib'))
	shutil.copyfile(lib_path_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Debug/pnglib_wind.lib'))
	shutil.copyfile(lib_path_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Release/pnglib_win.lib'))

	return True

def __build_win10(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_win10_x86_folder = os.path.join(working_directory_path, 'gen/build_win10_x86')
	build_win10_x64_folder = os.path.join(working_directory_path, 'gen/build_win10_x64')
	build_win10_arm_folder = os.path.join(working_directory_path, 'gen/build_win10_arm')

	# Build params
	# TODO: Do not use temporary zlib output, switch to repo/Libs/ instead
	solution_name = 'libpng.sln'
	target_name = 'png_static'
	cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(working_directory_path, '../zlib/gen/build_win10_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(working_directory_path, '../zlib/zlib_source/') ]
	cmake_win10_flags = cmake_flags + ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0' ]

	# Build all VS projects
	build_utils.cmake_generate_build_vs(build_win10_x86_folder, source_folder_path, 'Visual Studio 14 2015', solution_name, target_name, 'Win32', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_x64_folder, source_folder_path, 'Visual Studio 14 2015 Win64', solution_name, target_name, 'Win64', cmake_win10_flags)
	build_utils.cmake_generate_build_vs(build_win10_arm_folder, source_folder_path, 'Visual Studio 14 2015 ARM', solution_name, target_name, 'ARM', cmake_win10_flags)

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, 'Debug/libpng16_staticd.lib')
	lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, 'Release/libpng16_static.lib')
	lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, 'Debug/libpng16_staticd.lib')
	lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, 'Release/libpng16_static.lib')
	lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, 'Debug/libpng16_staticd.lib')
	lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, 'Release/libpng16_static.lib')

	shutil.copyfile(lib_path_win10_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Debug/pnglib_wind.lib'))
	shutil.copyfile(lib_path_win10_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Release/pnglib_win.lib'))
	shutil.copyfile(lib_path_win10_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Debug/pnglib_wind.lib'))
	shutil.copyfile(lib_path_win10_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Release/pnglib_win.lib'))
	shutil.copyfile(lib_path_win10_arm_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Debug/pnglib_wind.lib'))
	shutil.copyfile(lib_path_win10_arm_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Release/pnglib_win.lib'))

def __build_macos(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_folder_macos = os.path.join(working_directory_path, 'gen/build_macos')

	build_utils.cmake_generate_build_xcode(build_folder_macos, source_folder_path, 'Xcode', 'libpng.xcodeproj', 'png_static')

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_macos_release = os.path.join(build_folder_macos, 'Release/libpng16.a')

	shutil.copyfile(lib_path_macos_release, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libpng_macos.a'))

def __build_ios(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_folder_ios = os.path.join(working_directory_path, 'gen/build_ios')

	toolchain_filepath = os.path.join(root_project_path, 'Sources/CMake/Toolchains/ios.toolchain.cmake')

	build_utils.cmake_generate_build_xcode(build_folder_ios, source_folder_path, 'Xcode', 'libpng.xcodeproj', 'png_static', [ '-DCMAKE_TOOLCHAIN_FILE=' + toolchain_filepath ])
	
	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_ios_release = os.path.join(build_folder_ios, 'Release-iphoneos/libpng16.a')

	shutil.copyfile(lib_path_ios_release, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libpng_ios.a'))

def __build_android(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_android_armeabiv7a_folder = os.path.join(working_directory_path, 'gen/build_android_armeabiv7a')
	build_android_x86_folder = os.path.join(working_directory_path, 'gen/build_android_x86')

	toolchain_filepath = os.path.join(root_project_path, 'Sources/CMake/Toolchains/android.toolchain.cmake')
	android_ndk_folder_path = build_utils.get_android_ndk_folder_path(root_project_path)

	build_utils.cmake_generate_build_ndk(build_android_armeabiv7a_folder, source_folder_path, toolchain_filepath, android_ndk_folder_path, 'armeabi-v7a')
	build_utils.cmake_generate_build_ndk(build_android_x86_folder, source_folder_path, toolchain_filepath, android_ndk_folder_path, 'x86')

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_android_armeabiv7a = os.path.join(build_android_armeabiv7a_folder, 'libpng16.a')
	lib_path_android_x86 = os.path.join(build_android_x86_folder, 'libpng16.a')

	shutil.copyfile(lib_path_android_armeabiv7a, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libpng.a'))
	shutil.copyfile(lib_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libpng.a'))