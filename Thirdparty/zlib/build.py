import sys
import os
import shutil
import build_utils

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
	if target == 'win32':
		__build_win32(working_directory_path, root_project_path)
	elif target == 'win10':
		__build_win10(working_directory_path, root_project_path)
	else:
		return

def get_download_url():
	return 'http://zlib.net/zlib128.zip'

def __get_downloaded_archive_inner_dir():
	# Because zlib archive inner folder and archive file name do not match
	# If you change download link - change this one too
	return 'zlib-1.2.8'

def __download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'zlib_source')
	build_utils.download_and_extract(get_download_url(), working_directory_path, source_folder_path, __get_downloaded_archive_inner_dir())
	return source_folder_path

def __build_win32(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)

	build_x86_folder, build_x64_folder = build_utils.build_and_copy_libraries_win32_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'zlib.sln', 'zlibstatic',
		'zlibstaticd.lib', 'zlibstatic.lib',
		'zlib.lib', 'zlib.lib',
		'z.lib', 'z.lib')

	# Copy created configuration header to root folder
	# Required to use source folder as include path
	# TODO: get rid of this and copy to Libs/Include directly
	shutil.copyfile(os.path.join(build_x86_folder, 'zconf.h'), os.path.join(source_folder_path, 'zconf.h'))

	return True
	
def __build_win10(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	
	build_x86_folder, build_x64_folder, build_arm_folder = build_utils.build_and_copy_libraries_win10_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'zlib.sln', 'zlibstatic',
		'zlibstaticd.lib', 'zlibstatic.lib',
		'zlib.lib', 'zlib.lib',
		'zlib.lib', 'zlib.lib',
		'zlib.lib', 'zlib.lib')

	# Copy created configuration header to root folder
	# Required to use source folder as include path
	# TODO: get rid of this and copy to Libs/Include directly
	shutil.copyfile(os.path.join(build_x86_folder, 'zconf.h'), os.path.join(source_folder_path, 'zconf.h'))

	return True