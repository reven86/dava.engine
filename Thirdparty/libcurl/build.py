import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return []
	else:
		return ['macos', 'ios', 'android']

def get_dependencies_for_target(target):
	if target == 'android':
		return [ 'openssl' ]
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
	return { 'macos_and_ios': 'maintained by curl-ios-build-scripts (bundled)', 'others': 'https://curl.haxx.se/download/curl-7.50.3.tar.gz' }

def _download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libcurl_source')
	url = get_download_url()['others']
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))	
	return source_folder_path

def _build_macos(working_directory_path, root_project_path):
	build_curl_run_dir = os.path.join(working_directory_path, 'gen/build_osx')

	if not os.path.exists(build_curl_run_dir):
		os.makedirs(build_curl_run_dir)

	build_curl_args = ['./build_curl', '--arch', 'x86_64', '--run-dir', build_curl_run_dir]
	if (build_utils.verbose):
		build_curl_args.append('--verbose')

	build_utils.run_process(build_curl_args, process_cwd='curl-ios-build-scripts-master')

	output_path = os.path.join(build_curl_run_dir, 'curl/osx/lib/libcurl.a')

	shutil.copyfile(output_path, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/mac/libcurl_macos.a')))

	include_path = os.path.join(root_project_path, os.path.join('Libs/include/curl/iOS_MacOS'))
	build_utils.copy_files(os.path.join(build_curl_run_dir, 'curl/osx/include'), include_path, '*.h')

	return True

def _build_ios(working_directory_path, root_project_path):
	build_curl_run_dir = os.path.join(working_directory_path, 'gen/build_ios')

	if not os.path.exists(build_curl_run_dir):
		os.makedirs(build_curl_run_dir)

	build_curl_args = ['./build_curl', '--arch', 'armv7,armv7s,arm64', '--run-dir', build_curl_run_dir]
	if (build_utils.verbose):
		build_curl_args.append('--verbose')

	build_utils.run_process(build_curl_args, process_cwd='curl-ios-build-scripts-master')

	output_path = os.path.join(build_curl_run_dir, 'curl/ios-appstore/lib/libcurl.a')

	shutil.copyfile(output_path, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/ios/libcurl_ios.a')))

	include_path = os.path.join(root_project_path, os.path.join('Libs/include/curl/iOS_MacOS'))
	build_utils.copy_files(os.path.join(build_curl_run_dir, 'curl/ios-appstore/include'), include_path, '*.h')

	return True

def _build_android(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)

	env = os.environ.copy()
	original_path_var = env["PATH"]

	toolchain_path_arm = os.path.join(working_directory_path, 'gen/ndk_toolchain_arm')
	build_utils.android_ndk_make_toolchain(root_project_path, 'arm', 'android-14', 'darwin-x86_64', toolchain_path_arm)
	env['PATH'] = '{}:{}'.format(os.path.join(toolchain_path_arm, 'bin'), original_path_var)
	install_dir_arm = os.path.join(working_directory_path, 'gen/install_arm')
	configure_args = [ '--host=arm-linux-androideabi', '--disable-shared', '--with-ssl=' + os.path.abspath(os.path.join(working_directory_path, '../openssl/gen/install_arm/')) ]
	build_utils.build_with_autotools(source_folder_path, configure_args, install_dir_arm, env)

	toolchain_path_x86 = os.path.join(working_directory_path, 'gen/ndk_toolchain_x86')
	build_utils.android_ndk_make_toolchain(root_project_path, 'x86', 'android-14', 'darwin-x86_64', toolchain_path_x86)
	env['PATH'] = '{}:{}'.format(os.path.join(toolchain_path_x86, 'bin'), original_path_var)
	install_dir_arm = os.path.join(working_directory_path, 'gen/install_x86')
	configure_args = [ '--host=i686-linux-android', '--disable-shared', '--with-ssl=' + os.path.abspath(os.path.join(working_directory_path, '../openssl/gen/install_x86/')) ]
	build_utils.build_with_autotools(source_folder_path, configure_args, install_dir_arm, env)

	_copy_headers(source_folder_path, root_project_path, 'Others')

	return True

def _copy_headers(source_folder_path, root_project_path, target_folder):
	include_path = os.path.join(root_project_path, os.path.join('Libs/include/curl', target_folder))
	build_utils.copy_files(os.path.join(source_folder_path, 'include/curl'), include_path, '*.h')