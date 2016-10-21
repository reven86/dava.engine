#!/usr/bin/env python2.7
import urllib2
import zipfile
import os
import subprocess
import glob
import shutil
import tarfile
import sys
import build_config

verbose = False

def print_verbose(msg):
	if verbose:
		print msg

def download(url, file_name):
	path = os.path.dirname(file_name)
	if not os.path.exists(path):
		os.makedirs(path)

	u = urllib2.urlopen(url)
	f = open(file_name, 'wb')
	meta = u.info()
	file_size = int(meta.getheaders("Content-Length")[0])

	print "Downloading %s (%s bytes) from %s ..." % (file_name, file_size, url)

	file_size_dl = 0
	block_sz = 65536
	while True:
		buffer = u.read(block_sz)
		if not buffer:
			break

		file_size_dl += len(buffer)
		f.write(buffer)
		status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
		status = status + chr(8)*(len(status)+1)
		print status,

	f.close()

def download_if_doesnt_exist(url, file_name):
	if not os.path.exists(file_name):
		download(url, file_name)

def unzip_inplace(path):
	print "Unarchiving %s ..." % (path)

	extension = os.path.splitext(path)[1]
	if extension == '.zip':
		ref = zipfile.ZipFile(path, 'r')
	elif extension == '.gz':
		ref = tarfile.open(path, 'r')
	ref.extractall(os.path.dirname(path))
	ref.close()

def apply_patch(patch, working_dir = '.'):
	print "Applying patch %s" % patch
	proc = subprocess.Popen(["git", "apply", "--ignore-whitespace", patch], stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd = working_dir)
	for line in proc.stdout:
		print_verbose(line)
	proc.wait()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise

def build_vs(project, configuration, platform='Win32', target = None):
	print "Building %s for %s ..." % (project, configuration)
	args = ["c:/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe", project, "/p:Configuration="+configuration]
	if (not target is None):
		args.append('/target:' + target)
	proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	for line in proc.stdout:
		print_verbose(line)
	proc.wait()

	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise

def build_xcode_target(project, target, configuration):
	print "Building %s for %s ..." % (project, configuration)
	proc = subprocess.Popen(["xcodebuild", "-project", project, "-target", target, "-configuration", configuration, "build"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	for line in proc.stdout:
		print_verbose(line)
	proc.wait()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise

def build_xcode_alltargets(project, configuration):
	print "Building %s for %s ..." % (project, configuration)
	proc = subprocess.Popen(["xcodebuild", "-project", project, "-alltargets", "-configuration", configuration, "build"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	for line in proc.stdout:
		print_verbose(line)
	proc.wait()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise

def copy_files(from_dir, to_dir, wildcard):
	print "Copying %s from %s to %s" % (wildcard, from_dir, to_dir)
	for file in glob.glob(from_dir+"/"+wildcard):
		shutil.copy(file, to_dir)

def clean_copy_includes(from_dir, to_dir):
	print "Copying includes from %s to %s" % (from_dir, to_dir)
	if os.path.exists(to_dir) and os.path.isdir(to_dir):
		shutil.rmtree(to_dir)
	shutil.copytree(from_dir, to_dir)

def clear_files(dir, wildcard):
	print "Deleting %s in %s" % (wildcard, dir)
	map(os.remove, glob.glob(wildcard))

def copy_folder_recursive(src, dest, ignore=None):
    if os.path.isdir(src):
        if not os.path.isdir(dest):
            os.makedirs(dest)
        files = os.listdir(src)
        if ignore is not None:
            ignored = ignore(src, files)
        else:
            ignored = set()
        for f in files:
            if f not in ignored:
                copy_folder_recursive(os.path.join(src, f), 
                                    os.path.join(dest, f), 
                                    ignore)
    else:
        shutil.copyfile(src, dest)

def cmake_build(solution_folder_path, configuration):
	run_process("cmake --build . --config " + configuration, process_cwd=solution_folder_path, shell=True)

def cmake_generate(output_folder_path, src_folder_path, cmake_generator, cmake_additional_args = []):
	if not os.path.exists(output_folder_path):
		os.makedirs(output_folder_path)

	cmd = ['cmake', '-G', cmake_generator, src_folder_path]
	cmd.extend(cmake_additional_args)

	print 'Running CMake: {}, working directory: {}'.format(' '.join(cmd), output_folder_path)

	sp = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=output_folder_path)
	for line in sp.stdout:
		print_verbose(line)
	sp.wait()

def cmake_generate_build_vs(output_folder_path, src_folder_path, cmake_generator, sln_name, target, configuration, cmake_additional_args = []):
	cmake_generate(output_folder_path, src_folder_path, cmake_generator, cmake_additional_args)
	build_vs(os.path.join(output_folder_path, sln_name), 'Debug', configuration, target)
	build_vs(os.path.join(output_folder_path, sln_name), 'Release', configuration, target)

def cmake_generate_build_xcode(output_folder_path, src_folder_path, cmake_generator, project, target, cmake_additional_args = []):
	cmake_generate(output_folder_path, src_folder_path, cmake_generator, cmake_additional_args)
	build_xcode_target(os.path.join(output_folder_path, project), target, 'Release')

def cmake_generate_build_ndk(output_folder_path, src_folder_path, toolchain_filepath, android_ndk_path, abi):
	if not os.path.exists(output_folder_path):
		os.makedirs(output_folder_path)
 
	cmd = ['cmake', '-DCMAKE_TOOLCHAIN_FILE=' + toolchain_filepath, '-DANDROID_NDK=' + android_ndk_path, '-DCMAKE_BUILD_TYPE=Release', '-DANDROID_ABI=' + abi]
	if (sys.platform == 'win32'):
		cmd.extend(['-G', 'MinGW Makefiles', '-DCMAKE_MAKE_PROGRAM=' + os.path.join(android_ndk_path, 'prebuilt\\windows-x86_64\\bin\\make.exe')])

	cmd.append(src_folder_path)

	sp = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=output_folder_path)
	for line in sp.stdout:
		print_verbose(line)
	sp.wait()

	sp = subprocess.Popen(['cmake', '--build', '.'], stdout=subprocess.PIPE, cwd=output_folder_path)
	for line in sp.stdout:
		print_verbose(line)
	sp.wait()

def build_android_ndk(project_path, output_path, debug, ndk_additional_args = []):
	cmd = ['ndk-build', 'NDK_OUT=' + output_path]
	cmd.extend(ndk_additional_args)

	if debug:
		cmd.append('NDK_DEBUG=1')

	print 'Running ndk-build: {}, working directory: {}'.format(' '.join(cmd), project_path)

	if sys.platform == 'win32':
		sp = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=project_path, shell=True)
	else:
		# To avoid swallowing arguments by shell on macOS
		sp = subprocess.Popen(' '.join(cmd), stdout=subprocess.PIPE, cwd=project_path, shell=True)

	for line in sp.stdout:
		print_verbose(line)
	sp.wait()

def get_android_ndk_folder_path(root_project_path):
	config_file_path = os.path.join(root_project_path, 'DavaConfig.in')
	for line in open(config_file_path):
		splitted = line.strip().split('=')
		key = splitted[0].strip()
		if key == 'ANDROID_NDK':
			return splitted[1].strip()

	return None

def get_url_file_name(url):
	return url.split('/')[-1]

def get_url_file_name_no_ext(url):
	file_name = get_url_file_name(url)
	parts = file_name.split('.')
	# Handle special case for .tar.gz
	# TODO: a better way?
	last_index = -2 if parts[-1] == 'gz' else -1
	return '.'.join(parts[:last_index])

def download_and_extract(download_url, working_directory_path, result_folder_path, inner_dir_name = None):
	download_data = (download_url, result_folder_path)

	try:		
		if download_data in download_and_extract.cache:
			return result_folder_path
	except AttributeError:
		download_and_extract.cache = []

	# Download otherwise

	# Path to downloaded archive
	sources_filename = get_url_file_name(download_url)
	source_archive_filepath = os.path.join(working_directory_path, sources_filename)

	# Download & extract
	download(download_url, source_archive_filepath)
	unzip_inplace(source_archive_filepath)

	# Rename version-dependent folder name to simpler one
	# In case other builder will need to use this folder
	if inner_dir_name is not None:
		extracted_path = os.path.join(working_directory_path, inner_dir_name)
		if not os.path.exists(result_folder_path):
			shutil.move(extracted_path, result_folder_path)
		else:
			def deepmove(src, dst):
				if not os.path.exists(dst):
					os.mkdir(dst)
				for item in os.listdir(src):
					s = os.path.join(src, item)
					d = os.path.join(dst, item)
					if os.path.isdir(s):
						deepmove(s, d)
					else:
						shutil.move(s, d)
				os.rmdir(src)
			deepmove(extracted_path, result_folder_path)

	download_and_extract.cache.append(download_data)

def run_process(args, process_cwd='.', environment=None, shell=False):
	print 'running process: ' + ' '.join(args)
	for output_line in _run_process_iter(args, process_cwd, environment, shell):
		print_verbose(output_line)

def _run_process_iter(args, process_cwd='.', environment=None, shell=False):
	if environment is None:
		sp = subprocess.Popen(args, shell=shell, stdout=subprocess.PIPE, cwd=process_cwd)
	else:
		sp = subprocess.Popen(args, shell=shell, stdout=subprocess.PIPE, cwd=process_cwd, env=environment)

	stdout_lines = iter(sp.stdout.readline, '')
	for stdout_line in stdout_lines:
		yield stdout_line

	sp.stdout.close()
	return_code = sp.wait()

	if return_code != 0:
		raise subprocess.CalledProcessError(return_code, args)

def android_ndk_make_toolchain(root_project_path, arch, platform, system, install_dir):
	android_ndk_root = get_android_ndk_folder_path(root_project_path)

	exec_path = os.path.join(android_ndk_root, 'build/tools')

	cmd = ['sh', 'make-standalone-toolchain.sh', '--arch=' + arch, '--platform=' + platform, '--system=' + system, '--install-dir=' + install_dir, '--ndk-dir=' + android_ndk_root]
	run_process(cmd, process_cwd=exec_path)

def get_xcode_developer_path():
	sp = subprocess.Popen(['xcode-select', '-print-path'], stdout=subprocess.PIPE)
	stdout, stderr = sp.communicate()
	if stderr is None:
		return stdout.strip()
	else:
		print 'Error while getting xcode developer path: ' + stderr
		return None

def make_fat_darwin_binary(input_files_pathes, output_file_path):
	output_directory_path = os.path.dirname(output_file_path)
	if not os.path.exists(output_directory_path):
		os.makedirs(output_directory_path)

	args = ['lipo', '-output', output_file_path, '-create']
	args.extend(input_files_pathes)
	run_process(args)

# Default builders

def build_and_copy_libraries_win32_cmake(
		gen_folder_path, source_folder_path, root_project_path,
		solution_name, target_name,
		built_lib_name_debug, built_lib_name_release,
		result_lib_name_x86_debug, result_lib_name_x86_release,
		result_lib_name_x64_debug, result_lib_name_x64_release,
		cmake_additional_args = [], target_lib_subdir = ''):
	# Folders for the library to be built into
	build_x86_folder = os.path.join(gen_folder_path, 'build_win32_x86')
	build_x64_folder = os.path.join(gen_folder_path, 'build_win32_x64')

	# Generate & build
	cmake_generate_build_vs(build_x86_folder, source_folder_path, build_config.win32_x86_cmake_generator, solution_name, target_name, 'Win32', cmake_additional_args)
	cmake_generate_build_vs(build_x64_folder, source_folder_path, build_config.win32_x64_cmake_generator, solution_name, target_name, 'Win64', cmake_additional_args)
	
	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure	

	lib_path_x86_debug = os.path.join(build_x86_folder, os.path.join(target_lib_subdir, 'Debug', built_lib_name_debug))
	lib_path_x86_release = os.path.join(build_x86_folder, os.path.join(target_lib_subdir, 'Release', built_lib_name_release))
	lib_path_x64_debug = os.path.join(build_x64_folder, os.path.join(target_lib_subdir, 'Debug', built_lib_name_debug))
	lib_path_x64_release = os.path.join(build_x64_folder, os.path.join(target_lib_subdir, 'Release', built_lib_name_release))

	shutil.copyfile(lib_path_x86_debug, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win/x86/Debug', result_lib_name_x86_debug)))
	shutil.copyfile(lib_path_x86_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win/x86/Release', result_lib_name_x86_release)))
	shutil.copyfile(lib_path_x64_debug, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win/x64/Debug', result_lib_name_x64_debug)))
	shutil.copyfile(lib_path_x64_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win/x64/Release', result_lib_name_x64_release)))
	
	return (build_x86_folder, build_x64_folder)

def build_and_copy_libraries_win10_cmake(
		gen_folder_path, source_folder_path, root_project_path,
		solution_name, target_name,
		built_lib_name_debug, built_lib_name_release,
		result_lib_name_x86_debug, result_lib_name_x86_release,
		result_lib_name_x64_debug, result_lib_name_x64_release,
		result_lib_name_arm_debug, result_lib_name_arm_release,
		cmake_additional_args = []):
	# Folders for the library to be built into
	build_win10_x86_folder = os.path.join(gen_folder_path, 'build_win10_x86')
	build_win10_x64_folder = os.path.join(gen_folder_path, 'build_win10_x64')
	build_win10_arm_folder = os.path.join(gen_folder_path, 'build_win10_arm')

	cmake_win10_flags = ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0']
	cmake_additional_args.extend(cmake_win10_flags)

	# Generate & build
	cmake_generate_build_vs(build_win10_x86_folder, source_folder_path, build_config.win10_x86_cmake_generator, solution_name, target_name, 'Win32', cmake_additional_args)
	cmake_generate_build_vs(build_win10_x64_folder, source_folder_path, build_config.win10_x64_cmake_generator, solution_name, target_name, 'Win64', cmake_additional_args)
	cmake_generate_build_vs(build_win10_arm_folder, source_folder_path, build_config.win10_arm_cmake_generator, solution_name, target_name, 'ARM', cmake_additional_args)

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure	

	lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, os.path.join('Debug', built_lib_name_debug))
	lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, os.path.join('Release', built_lib_name_release))
	lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, os.path.join('Debug', built_lib_name_debug))
	lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, os.path.join('Release', built_lib_name_release))
	lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, os.path.join('Debug', built_lib_name_debug))
	lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, os.path.join('Release', built_lib_name_release))

	shutil.copyfile(lib_path_win10_x86_debug, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win10/Win32/Debug', result_lib_name_x86_debug)))
	shutil.copyfile(lib_path_win10_x86_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win10/Win32/Release', result_lib_name_x86_release)))
	shutil.copyfile(lib_path_win10_x64_debug, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win10/x64/Debug', result_lib_name_x64_debug)))
	shutil.copyfile(lib_path_win10_x64_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win10/x64/Release', result_lib_name_x64_release)))
	shutil.copyfile(lib_path_win10_arm_debug, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win10/arm/Debug', result_lib_name_arm_debug)))
	shutil.copyfile(lib_path_win10_arm_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/win10/arm/Release', result_lib_name_arm_release)))

	return (build_win10_x86_folder, build_win10_x64_folder, build_win10_arm_folder)

def build_and_copy_libraries_macos_cmake(
		gen_folder_path, source_folder_path, root_project_path,
		project_name, target_name,
		built_lib_name_release,
		result_lib_name_release,
		cmake_additional_args = [], target_lib_subdir=''):
	build_folder_macos = os.path.join(gen_folder_path, 'build_macos')

	cmake_generate_build_xcode(build_folder_macos, source_folder_path, build_config.macos_cmake_generator, project_name, target_name, cmake_additional_args)

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_macos_release = os.path.join(build_folder_macos, os.path.join(target_lib_subdir, 'Release', built_lib_name_release))

	shutil.copyfile(lib_path_macos_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/mac', result_lib_name_release)))

	return build_folder_macos

def build_and_copy_libraries_ios_cmake(
		gen_folder_path, source_folder_path, root_project_path,
		project_name, target_name,
		built_lib_name_release,
		result_lib_name_release,
		cmake_additional_args = []):
	build_folder_ios = os.path.join(gen_folder_path, 'build_ios')

	toolchain_filepath = os.path.join(root_project_path, 'Sources/CMake/Toolchains/ios.toolchain.cmake')
	cmake_additional_args.append('-DCMAKE_TOOLCHAIN_FILE=' + toolchain_filepath)

	cmake_generate_build_xcode(build_folder_ios, source_folder_path, build_config.macos_cmake_generator, project_name, target_name, cmake_additional_args)
	
	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_ios_release = os.path.join(build_folder_ios, os.path.join('Release-iphoneos', built_lib_name_release))

	shutil.copyfile(lib_path_ios_release, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/ios', result_lib_name_release)))

	return build_folder_ios

def build_and_copy_libraries_android_cmake(
		gen_folder_path, source_folder_path, root_project_path,
		built_lib_name_release,
		result_lib_name_release,
		cmake_additional_args = []):
	build_android_armeabiv7a_folder = os.path.join(gen_folder_path, 'build_android_armeabiv7a')
	build_android_x86_folder = os.path.join(gen_folder_path, 'build_android_x86')

	toolchain_filepath = os.path.join(root_project_path, 'Sources/CMake/Toolchains/android.toolchain.cmake')
	android_ndk_folder_path = get_android_ndk_folder_path(root_project_path)

	cmake_generate_build_ndk(build_android_armeabiv7a_folder, source_folder_path, toolchain_filepath, android_ndk_folder_path, 'armeabi-v7a')
	cmake_generate_build_ndk(build_android_x86_folder, source_folder_path, toolchain_filepath, android_ndk_folder_path, 'x86')

	# Move built files into Libs/lib_CMake
	# TODO: update pathes after switching to new folders structure

	lib_path_android_armeabiv7a = os.path.join(build_android_armeabiv7a_folder, built_lib_name_release)
	lib_path_android_x86 = os.path.join(build_android_x86_folder, built_lib_name_release)

	shutil.copyfile(lib_path_android_armeabiv7a, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/android/armeabi-v7a', result_lib_name_release)))
	shutil.copyfile(lib_path_android_x86, os.path.join(root_project_path, os.path.join('Libs/lib_CMake/android/x86', result_lib_name_release)))

	return (build_android_x86_folder, build_android_armeabiv7a_folder)

def build_with_autotools(source_folder_path, configure_args, install_dir, env=None, configure_exec_name='configure'):
	cmd = ['./{}'.format(configure_exec_name)]
	cmd.extend(configure_args)
	if install_dir is not None:
		if not os.path.exists(install_dir):
			os.makedirs(install_dir)
		cmd.append('--prefix=' + install_dir)

	run_process(cmd, process_cwd=source_folder_path, environment=env)

	cmd = ['make', 'all']
	run_process(cmd, process_cwd=source_folder_path, environment=env)

	if install_dir is not None:
		cmd = ['make', 'install']
		run_process(cmd, process_cwd=source_folder_path, environment=env)

		cmd = ['make', 'clean']
		run_process(cmd, process_cwd=source_folder_path, environment=env)
