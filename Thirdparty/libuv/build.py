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
	return 'http://dist.libuv.org/dist/v1.9.1/libuv-v1.9.1.tar.gz'

def _download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libuv_source')
	url = get_download_url()
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))	
	return source_folder_path

def _build_macos(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	env = build_utils.get_autotools_macos_env()

	install_dir_macos = os.path.join(working_directory_path, 'gen/install_macos')
	build_utils.run_process(['sh', 'autogen.sh'], process_cwd=source_folder_path, environment=env)
	build_utils.build_with_autotools(source_folder_path, ['--host=x86_64-apple-darwin', '--disable-shared', '--enable-static'], install_dir_macos, env=env)

	lib_path = os.path.join(install_dir_macos, 'lib/libuv.a')
	shutil.copyfile(lib_path, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libuv_macos.a'))

	_copy_headers_from_install(install_dir_macos, root_project_path)

def _build_ios(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	env = build_utils.get_autotools_ios_env()

	install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
	build_utils.run_process(['sh', 'autogen.sh'], process_cwd=source_folder_path, environment=env)
	build_utils.build_with_autotools(source_folder_path, ['--host=armv7-apple-darwin', '--disable-shared', '--enable-static'], install_dir_ios, env=env)

	lib_path = os.path.join(install_dir_ios, 'lib/libuv.a')
	shutil.copyfile(lib_path, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libuv_ios.a'))

	_copy_headers_from_install(install_dir_ios, root_project_path)

def _build_android(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	env_arm = build_utils.get_autotools_android_arm_env(root_project_path)
	install_dir_android_arm = os.path.join(working_directory_path, 'gen/install_android_arm')
	build_utils.run_process(['sh', 'autogen.sh'], process_cwd=source_folder_path, environment=env_arm)
	build_utils.build_with_autotools(source_folder_path, ['--host=arm-linux-androideabi', '--disable-shared', '--enable-static'], install_dir_android_arm, env=env_arm)

	env_x86 = build_utils.get_autotools_android_x86_env(root_project_path)
	install_dir_android_x86 = os.path.join(working_directory_path, 'gen/install_android_x86')
	build_utils.run_process(['sh', 'autogen.sh'], process_cwd=source_folder_path, environment=env_x86)
	build_utils.build_with_autotools(source_folder_path, ['--host=i686-linux-android', '--disable-shared', '--enable-static'], install_dir_android_x86, env=env_x86)

	lib_path_arm = os.path.join(install_dir_android_arm, 'lib/libuv.a')
	shutil.copyfile(lib_path_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libuv.a'))

	lib_path_x86 = os.path.join(install_dir_android_x86, 'lib/libuv.a')
	shutil.copyfile(lib_path_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libuv.a'))

	_copy_headers_from_install(install_dir_android_arm, root_project_path)

def _copy_headers_from_install(install_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/libuv')
	build_utils.copy_folder_recursive(os.path.join(install_folder_path, 'include'), include_path)

def _copy_headers(source_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/libuv')
	build_utils.copy_files(os.path.join(source_folder_path, 'include'), include_path, '*.h')