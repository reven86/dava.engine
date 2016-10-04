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
	if target == 'win32':
		return _build_win32(working_directory_path, root_project_path)
	elif target == 'win10':
		return _build_win10(working_directory_path, root_project_path)
	elif target == 'macos':
		return _build_macos(working_directory_path, root_project_path)
	elif target == 'ios':
		return _build_ios(working_directory_path, root_project_path)
	elif target == 'android':
		return _build_android(working_directory_path, root_project_path)

def get_download_url():
	return {'win32' : 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/lpng1625.zip', 'others' : 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.25.tar.gz'}

def _download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libpng_source')
	if sys.platform == 'win32':
		url = get_download_url()['win32']
	else:
		url = get_download_url()['others']
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))
	return source_folder_path

def _patch_sources(source_folder_path, working_directory_path):
	# Skip if we've already did the job once
	try:
		if _patch_sources.did:
			return
	except AttributeError:
		pass

	# Apply fixes
	build_utils.apply_patch(os.path.abspath('patch.diff'), working_directory_path)

	# Add configuration file to source folder
	# It is used to generate additional header & source files
	shutil.copyfile('pngusr.dfa', os.path.join(source_folder_path, 'pngusr.dfa'))

	_patch_sources.did = True

def _build_win32(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(working_directory_path, '../zlib/gen/build_win32_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(working_directory_path, '../zlib/zlib_source/') ]

	build_utils.build_and_copy_libraries_win32_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.sln', 'png_static',
		'libpng16_staticd.lib', 'libpng16_static.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		cmake_flags)

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_win10(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(working_directory_path, '../zlib/gen/build_win10_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(working_directory_path, '../zlib/zlib_source/') ]

	build_utils.build_and_copy_libraries_win10_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.sln', 'png_static',
		'libpng16_staticd.lib', 'libpng16_static.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		'pnglib_wind.lib', 'pnglib_win.lib',
		cmake_flags)

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_macos(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_macos_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.xcodeproj', 'png_static',
		'libpng16.a',
		'libpng_macos.a')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_ios(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_ios_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng.xcodeproj', 'png_static',
		'libpng16.a',
		'libpng_ios.a')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_android(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_android_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libpng16.a',
		'libpng.a')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _copy_headers(source_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/libpng')
	build_utils.copy_files(source_folder_path, include_path, '*.h')