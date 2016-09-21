import sys
import os
import shutil
import build_utils

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
		return __build_all_on_current_platform(working_directory_path, root_project_path)
	elif target == 'win32':
		return __build_win32(working_directory_path, root_project_path)
	elif target == 'win10':
		return __build_win10(working_directory_path, root_project_path)
	elif target == 'macos':
		return __build_macos(working_directory_path, root_project_path)
	elif target == 'ios':
		return __build_ios(working_directory_path, root_project_path)
	elif target == 'android':
		return __build_android(working_directory_path, root_project_path)

def get_download_url():
	return {'win32' : 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/lpng1625.zip', 'others' : 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.25.tar.gz'}

def __get_downloaded_archive_inner_dir():
	if sys.platform == 'win32':
		return 'lpng1625'
	else:
		return 'libpng-1.6.25'

def __download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libpng_source')
	if sys.platform == 'win32':
		download_link = get_download_url()['win32']
	else:
		download_link = get_download_url()['others']
	build_utils.download_and_extract(download_link, working_directory_path, source_folder_path, __get_downloaded_archive_inner_dir())
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
		return (__build_win32(working_directory_path, root_project_path) and
				__build_win10(working_directory_path, root_project_path) and
				__build_android(working_directory_path, root_project_path))
	else:
		return (__build_macos(working_directory_path, root_project_path) and
				__build_ios(working_directory_path, root_project_path) and
				__build_android(working_directory_path, root_project_path))

def __build_win32(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(working_directory_path, '../zlib/gen/build_win32_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(working_directory_path, '../zlib/zlib_source/') ]

	build_utils.build_and_copy_libraries_win32_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.sln', 'png_static',
		'libpng16_staticd.lib', 'libpng16_static.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		cmake_flags)

	return True

def __build_win10(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(working_directory_path, '../zlib/gen/build_win10_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(working_directory_path, '../zlib/zlib_source/') ]

	build_utils.build_and_copy_libraries_win10_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.sln', 'png_static',
		'libpng16_staticd.lib', 'libpng16_static.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		cmake_flags)

	return True

def __build_macos(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_macos_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.xcodeproj', 'png_static',
		'libpng16.a',
		'libpng_macos.a')

	return True

def __build_ios(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_ios_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.xcodeproj', 'png_static',
		'libpng16.a',
		'libpng_ios.a')

	return True

def __build_android(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_android_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng16.a',
		'libpng.a')

	return True