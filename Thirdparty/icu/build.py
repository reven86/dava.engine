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
	return 'http://download.icu-project.org/files/icu4c/57.1/icu4c-57_1-src.tgz'

def _download_and_extract(working_directory_path, source_folder_path_prefix=''):
	source_folder_path = os.path.join(working_directory_path, 'icu_source' + source_folder_path_prefix)
	url = get_download_url()
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, 'icu')	
	return source_folder_path

def _build_macos(working_directory_path, root_project_path):
	try:
		if _build_macos.did:
			return
	except AttributeError:
		pass

	source_folder_path = _download_and_extract(working_directory_path, '_macos')

	install_dir_macos = os.path.join(working_directory_path, 'gen/install_macos')
	build_utils.build_with_autotools(os.path.join(source_folder_path, 'source'), ['--host=x86_64-apple-darwin', '--disable-shared', '--enable-static'], install_dir_macos, env=build_utils.get_autotools_macos_env(), postclean=False)

	_build_macos.did = True

def _build_ios(working_directory_path, root_project_path):
	_build_macos(working_directory_path, root_project_path)

	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
	build_utils.build_with_autotools(os.path.join(source_folder_path, 'source'), ['--with-cross-build=' + os.path.abspath(source_folder_path + '_macos/source'), '--host=armv7-apple-darwin', '--disable-shared', '--enable-static'], install_dir_ios, env=build_utils.get_autotools_ios_env())
	
def _build_android(working_directory_path, root_project_path):
	_build_macos(working_directory_path, root_project_path)

	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_android_arm = os.path.join(working_directory_path, 'gen/install_android_arm')
	build_utils.build_with_autotools(os.path.join(source_folder_path, 'source'), ['--with-cross-build=' + os.path.abspath(source_folder_path + '_macos/source'), '--host=arm-linux-androideabi', '--disable-shared', '--enable-static'], install_dir_android_arm, env=build_utils.get_autotools_android_arm_env(root_project_path, enable_stl=True))

	install_dir_android_x86 = os.path.join(working_directory_path, 'gen/install_android_x86')
	build_utils.build_with_autotools(os.path.join(source_folder_path, 'source'), ['--with-cross-build=' + os.path.abspath(source_folder_path + '_macos/source'), '--host=i686-linux-android', '--disable-shared', '--enable-static'], install_dir_android_x86, env=build_utils.get_autotools_android_x86_env(root_project_path, enable_stl=True))

# TODO: Add copying headers when switching to new directories structure