import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return ['win32', 'win10', 'android']
	else:
		return ['macos', 'ios', 'android']

def get_dependencies_for_target(target):
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
	return { 'macos_and_ios': 'maintained by curl-ios-build-scripts (bundled)' }

def __build_macos(working_directory_path, root_project_path):
	build_curl_run_dir = os.path.join(working_directory_path, 'gen/build_osx')

	if not os.path.exists(build_curl_run_dir):
		os.makedirs(build_curl_run_dir)

	build_curl_args = ['./build_curl', '--arch', 'x86_64', '--run-dir', build_curl_run_dir]
	if (build_utils.verbose):
		build_curl_args.append('--verbose')

	build_utils.run_process(build_curl_args, process_cwd='curl-ios-build-scripts-master')

	output_path = os.path.join(build_curl_run_dir, 'curl/osx/lib/libcurl.a')

	shutil.copyfile(output_path, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/mac/libcurl_macos.a')))

	return True

def __build_ios(working_directory_path, root_project_path):
	build_curl_run_dir = os.path.join(working_directory_path, 'gen/build_ios')

	if not os.path.exists(build_curl_run_dir):
		os.makedirs(build_curl_run_dir)

	build_curl_args = ['./build_curl', '--arch', 'armv7,armv7s,arm64', '--run-dir', build_curl_run_dir]
	if (build_utils.verbose):
		build_curl_args.append('--verbose')

	build_utils.run_process(build_curl_args, process_cwd='curl-ios-build-scripts-master')

	output_path = os.path.join(build_curl_run_dir, 'curl/ios-appstore/lib/libcurl.a')

	shutil.copyfile(output_path, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/ios/libcurl_ios.a')))

	return True