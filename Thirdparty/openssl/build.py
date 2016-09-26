import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return []
	else:
		return ['macos', 'ios', 'android']

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

configure_args = ['no-whirlpool', 'no-asm', 'no-cast', 'no-idea', 'no-camellia', 'no-comp', 'no-hw', 'no-engine']

def __build_macos(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	install_dir = os.path.join(working_directory_path, 'gen/install_macos')
	macos_configure_args = list(configure_args)
	macos_configure_args.insert(0, 'darwin64-x86_64-cc')
	build_utils.build_with_autotools(source_folder_path, macos_configure_args, install_dir, configure_exec_name='Configure')
	
	libssl_path = os.path.join(install_dir, 'lib/libssl.a')
	libcrypto_path = os.path.join(install_dir, 'lib/libcrypto.a')
	shutil.copyfile(libssl_path, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libssl.a'))
	shutil.copyfile(libcrypto_path, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libcrypto.a'))

	__copy_headers(install_dir, root_project_path, 'mac')

def __build_ios(working_directory_path, root_project_path):
	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	install_dir_armv7 = os.path.join(working_directory_path, 'gen/install_ios_armv7')
	ios_configure_args = list(configure_args)
	ios_configure_args.insert(0, 'ios-cross')
	build_utils.build_with_autotools(source_folder_path, ios_configure_args, install_dir_armv7, configure_exec_name='Configure', env=__get_ios_env())

	install_dir_arm64 = os.path.join(working_directory_path, 'gen/install_ios_arm64')
	ios_configure_args = list(configure_args)
	ios_configure_args.insert(0, 'ios64-cross')
	build_utils.build_with_autotools(source_folder_path, ios_configure_args, install_dir_arm64, configure_exec_name='Configure', env=__get_ios_env())

	libssl_fat_path = os.path.join(working_directory_path, 'gen/fat_ios/libssl.a')
	libcrypto_fat_path = os.path.join(working_directory_path, 'gen/fat_ios/libcrypto.a')
	build_utils.make_fat_darwin_binary([os.path.join(install_dir_armv7, 'lib/libssl.a'), os.path.join(install_dir_arm64, 'lib/libssl.a')], libssl_fat_path)
	build_utils.make_fat_darwin_binary([os.path.join(install_dir_armv7, 'lib/libcrypto.a'), os.path.join(install_dir_arm64, 'lib/libcrypto.a')], libcrypto_fat_path)

	shutil.copyfile(libssl_fat_path, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libssl.a'))
	shutil.copyfile(libcrypto_fat_path, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libcrypto.a'))

	__copy_headers(install_dir_armv7, root_project_path, 'ios')

def __build_android(working_directory_path, root_project_path):
	# https://wiki.openssl.org/index.php/Android

	source_folder_path = __download_and_extract(working_directory_path)
	__patch_sources(source_folder_path, working_directory_path)

	install_dir_arm = os.path.join(working_directory_path, 'gen/install_android_arm')
	build_utils.build_with_autotools(source_folder_path, configure_args, install_dir_arm, configure_exec_name='config', env=__get_android_env_arm(source_folder_path, root_project_path))

	install_dir_x86 = os.path.join(working_directory_path, 'gen/install_android_x86')
	build_utils.build_with_autotools(source_folder_path, configure_args, install_dir_x86, configure_exec_name='config', env=__get_android_env_x86(source_folder_path, root_project_path))

	libssl_path_android_arm = os.path.join(install_dir_arm, 'lib/libssl.a')
	libcrypto_path_android_arm = os.path.join(install_dir_arm, 'lib/libcrypto.a')
	libssl_path_android_x86 = os.path.join(install_dir_x86, 'lib/libssl.a')
	libcrypto_path_android_x86 = os.path.join(install_dir_x86, 'lib/libcrypto.a')

	shutil.copyfile(libssl_path_android_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libssl.a'))
	shutil.copyfile(libcrypto_path_android_arm, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libcrypto.a'))
	shutil.copyfile(libssl_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libssl.a'))
	shutil.copyfile(libcrypto_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libcrypto.a'))

	__copy_headers(install_dir_arm, root_project_path, 'android')

	return True

def __get_ios_env():
	xcode_developer_path = build_utils.get_xcode_developer_path()

	env = os.environ.copy()
	env['CROSS_COMPILE'] = os.path.join(xcode_developer_path, 'Toolchains/XcodeDefault.xctoolchain/usr/bin/')
	env['CROSS_TOP'] = os.path.join(xcode_developer_path, 'Platforms/iPhoneOS.platform/Developer/')
	env['CROSS_SDK'] = 'iPhoneOS.sdk'

	return env

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

def __copy_headers(install_path, root_project_path, target_folder):
	include_subpath = os.path.join(os.path.join('Libs/openssl/include', target_folder), 'openssl')
	include_path = os.path.join(root_project_path, include_subpath)
	build_utils.copy_files(os.path.join(install_path, 'include'), include_path, '*.h')