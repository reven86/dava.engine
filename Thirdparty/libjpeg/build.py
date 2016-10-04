import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return ['win32', 'win10']
	else:
		return ['macos', 'ios', 'android']

def get_dependencies_for_target(target):
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
	return 'http://www.ijg.org/files/jpegsrc.v9b.tar.gz'

def _get_downloaded_archive_inner_dir():
	# Because archive inner folder and archive file name do not match
	# If you change download link - change this one too
	return 'jpeg-9b'

def _download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libjpeg_source')
	url = get_download_url()
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, _get_downloaded_archive_inner_dir())	
	return source_folder_path

def _patch_sources(source_folder_path, working_directory_path):
	try:
		if _patch_sources.did:
			return
	except AttributeError:
		pass

	_patch_sources.did = True

def _build_macos(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_macos = os.path.join(working_directory_path, 'gen/install_macos')
	build_utils.build_with_autotools(source_folder_path, ['--host=x86_64-apple-darwin', '--disable-shared', '--enable-static'], install_dir_macos, env=build_utils.get_autotools_macos_env())

	libunibreak_lib_path = os.path.join(install_dir_macos, 'lib/libjpeg.a')
	shutil.copyfile(libunibreak_lib_path, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libjpeg_macos.a'))

	_copy_headers_from_install(install_dir_macos, root_project_path)

def _build_ios(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
	build_utils.build_with_autotools(source_folder_path, ['--host=armv7-apple-darwin', '--disable-shared', '--enable-static'], install_dir_ios, env=build_utils.get_autotools_ios_env())
	
	libunibreak_lib_path = os.path.join(install_dir_ios, 'lib/libjpeg.a')
	shutil.copyfile(libunibreak_lib_path, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libjpeg_ios.a'))

	_copy_headers_from_install(install_dir_ios, root_project_path)

def _build_android(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_android_arm = os.path.join(working_directory_path, 'gen/install_android_arm')
	build_utils.build_with_autotools(source_folder_path, ['--host=arm-linux-androideabi', '--disable-shared', '--enable-static'], install_dir_android_arm, env=build_utils.get_autotools_android_arm_env(root_project_path))

	install_dir_android_x86 = os.path.join(working_directory_path, 'gen/install_android_x86')
	build_utils.build_with_autotools(source_folder_path, ['--host=i686-linux-android', '--disable-shared', '--enable-static'], install_dir_android_x86, env=build_utils.get_autotools_android_x86_env(root_project_path))

	libunibreak_lib_path_arm = os.path.join(install_dir_android_arm, 'lib/libjpeg.a')
	shutil.copyfile(libunibreak_lib_path_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libjpeg.a'))

	libunibreak_lib_path_x86 = os.path.join(install_dir_android_x86, 'lib/libjpeg.a')
	shutil.copyfile(libunibreak_lib_path_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libjpeg.a'))

	_copy_headers_from_install(install_dir_android_arm, root_project_path)

def _copy_headers_from_install(install_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/libjpeg')
	build_utils.copy_folder_recursive(os.path.join(install_folder_path, 'include'), include_path)

def _copy_headers(source_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/libjpeg')
	build_utils.copy_files(source_folder_path, include_path, '*.h')