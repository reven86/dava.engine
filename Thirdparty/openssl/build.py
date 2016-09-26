import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return []
	else:
		return ['android']

def get_dependencies_for_target(target):
	return []

def get_supported_build_platforms():
	return ['darwin']

def build_for_target(target, working_directory_path, root_project_path):
	if target == 'win32':
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
	return 'https://www.openssl.org/source/openssl-1.1.0a.tar.gz'

def __download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'openssl_source')
	url = get_download_url()
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))	
	return source_folder_path

def __patch_sources(source_folder_path, working_directory_path):
	try:
		if __patch_sources.did:
			return
	except AttributeError:
		pass

	build_utils.apply_patch(os.path.abspath('patch.diff'), working_directory_path)

	__patch_sources.did = True

def __build_android(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	configure_args = ['shared', 'no-whirlpool', 'no-asm', 'no-cast', 'no-idea', 'no-camellia', 'no-comp', 'no-hw', 'no-engine']

	install_dir_arm = os.path.join(working_directory_path, 'gen/install_arm')
	build_utils.build_with_autotools(source_folder_path, configure_args, install_dir_arm, configure_exec_name='config', env=__get_android_env_arm(source_folder_path, root_project_path))

	install_dir_x86 = os.path.join(working_directory_path, 'gen/install_x86')
	build_utils.build_with_autotools(source_folder_path, configure_args, install_dir_x86, configure_exec_name='config', env=__get_android_env_x86(source_folder_path, root_project_path))

	libssl_path_android_arm = os.path.join(install_dir_arm, 'lib/libssl.a')
	libcrypto_path_android_arm = os.path.join(install_dir_arm, 'lib/libcrypto.a')
	libssl_path_android_x86 = os.path.join(install_dir_x86, 'lib/libssl.a')
	libcrypto_path_android_x86 = os.path.join(install_dir_x86, 'lib/libcrypto.a')

	shutil.copyfile(libssl_path_android_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libssl.a'))
	shutil.copyfile(libcrypto_path_android_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libcrypto.a'))
	shutil.copyfile(libssl_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libssl.a'))
	shutil.copyfile(libcrypto_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libcrypto.a'))

	return True

def __get_android_env(source_folder_path, root_project_path, android_target, machine, arch, toolchain_folder, cross_compile, crystax_libs_folder):
	# Python version of setenv.sh from https://wiki.openssl.org/index.php/Android

	android_ndk_root = build_utils.get_android_ndk_folder_path(root_project_path)
	platform_path = '{}/platforms/{}/arch-{}'.format(android_ndk_root, android_target, arch)
	eabi_path = '{}/toolchains/{}/prebuilt/darwin-x86_64/bin'.format(android_ndk_root, toolchain_folder)
	crystax_libs_cflag = '-L{}/sources/crystax/libs/{}/'.format(android_ndk_root, crystax_libs_folder)
	fips_sig_path = '{}/util/incore'.format(source_folder_path)

	env = os.environ.copy()
	env['SYSTEM'] = 'android'
	env['MACHINE'] = machine
	env['ARCH'] = arch
	env['CROSS_SYSROOT'] = platform_path
	env['CROSS_COMPILE'] = cross_compile
	env['PATH'] = '{}:{}'.format(eabi_path, env['PATH'])
	env['CRYSTAX_LDFLAGS'] = crystax_libs_cflag
	env['FIPS_SIG'] = fips_sig_path

	return env

def __get_android_env_arm(source_folder_path, root_project_path):
	return __get_android_env(source_folder_path, root_project_path, 'android-9', 'armv7', 'arm', 'arm-linux-androideabi-4.9', 'arm-linux-androideabi-', 'armeabi-v7a')

def __get_android_env_x86(source_folder_path, root_project_path):
	return __get_android_env(source_folder_path, root_project_path, 'android-9', 'i686', 'x86', 'x86-4.9', 'i686-linux-android-', 'x86')