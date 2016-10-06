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
	return 'ftp://xmlsoft.org/libxml2/libxml2-2.9.4.tar.gz'

def _download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libxml2_source')
	url = get_download_url()
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))	
	return source_folder_path

def _patch_sources_android(source_folder_path):
	try:
		if source_folder_path in _patch_sources_android.cache:
			return
	except AttributeError:
		_patch_sources_android.cache = []
		pass

	# Glob.h is required for android since Android NDK doesn't include one
	# Glob.o will be compiled and added to linker input
	shutil.copyfile('glob.h', os.path.join(source_folder_path, 'glob.h'))

	_patch_sources_android.cache.append(source_folder_path)

def _build_macos(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_macos = os.path.join(working_directory_path, 'gen/install_macos')
	build_utils.build_with_autotools(source_folder_path, ['--host=x86_64-apple-darwin', '--disable-shared', '--enable-static'], install_dir_macos, env=build_utils.get_autotools_macos_env())

	lib_path = os.path.join(install_dir_macos, 'lib/libxml2.a')
	shutil.copyfile(lib_path, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libxml_macos.a'))

	_copy_headers_from_install(install_dir_macos, root_project_path)

def _build_ios(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
	build_utils.build_with_autotools(source_folder_path, ['--host=armv7-apple-darwin', '--disable-shared', '--enable-static'], install_dir_ios, env=build_utils.get_autotools_ios_env())

	lib_path = os.path.join(install_dir_ios, 'lib/libxml2.a')
	shutil.copyfile(lib_path, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libxml_ios.a'))

	_copy_headers_from_install(install_dir_ios, root_project_path)

def _build_android(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources_android(source_folder_path)

	# Libxml tests requires glob header to be available, but android ndk doesn't include one
	# To work around this, glob.c & glob.h are included
	# glob.c should be compiled with according ndk compiler and added to linker input
	# Files are taken from this discussion: https://groups.google.com/forum/#!topic/android-ndk/vSH6MWPD0Vk

	gen_folder_path = os.path.join(working_directory_path, 'gen')
	glob_obj_file_path = os.path.join(gen_folder_path, 'glob.o')
	if not os.path.exists(gen_folder_path):
		os.makedirs(gen_folder_path)

	arm_env = build_utils.get_autotools_android_arm_env(root_project_path)
	install_dir_android_arm = os.path.join(working_directory_path, 'gen/install_android_arm')
	_compile_glob(arm_env, glob_obj_file_path)
	arm_env['LIBS'] = glob_obj_file_path
	build_utils.build_with_autotools(source_folder_path, ['--host=arm-linux-androideabi', '--disable-shared', '--enable-static'], install_dir_android_arm, env=arm_env)


	x86_env = build_utils.get_autotools_android_x86_env(root_project_path)
	install_dir_android_x86 = os.path.join(working_directory_path, 'gen/install_android_x86')
	_compile_glob(x86_env, glob_obj_file_path)
	x86_env['LIBS'] = glob_obj_file_path
	build_utils.build_with_autotools(source_folder_path, ['--host=i686-linux-android', '--disable-shared', '--enable-static'], install_dir_android_x86, env=x86_env)

	lib_path_arm = os.path.join(install_dir_android_arm, 'lib/libxml2.a')
	shutil.copyfile(lib_path_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libxml.a'))
	lib_path_x86 = os.path.join(install_dir_android_x86, 'lib/libxml2.a')
	shutil.copyfile(lib_path_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libxml.a'))

	_copy_headers_from_install(install_dir_android_x86, root_project_path)

def _compile_glob(env, output_file_path):
	cmd = [env['CC'], '-c', '-I.', 'glob.c', '-o', output_file_path]
	cmd.extend(env['CFLAGS'].split())
	build_utils.run_process(cmd, environment=env)

def _copy_headers_from_install(install_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/libxml')
	build_utils.copy_folder_recursive(os.path.join(install_folder_path, 'include/libxml2/libxml'), include_path)
