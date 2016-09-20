#!/usr/bin/env python2.7
import urllib2
import zipfile
import os
import subprocess
import glob
import shutil
import tarfile
import sys

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
	shutil.rmtree(to_dir)
	shutil.copytree(from_dir, to_dir)

def clear_files(dir, wildcard):
	print "Deleting %s in %s" % (wildcard, dir)
	map(os.remove, glob.glob(wildcard))

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
	build_xcode_target(os.path.join(output_folder_path, project), target, 'Debug')
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

def download_and_extract(download_url, working_directory_path, result_folder_path, inner_dir_name = None):
	download_data = (download_url, result_folder_path)

	try:		
		if download_data in download_and_extract.cache:
			return result_folder_path
	except AttributeError:
		download_and_extract.cache = []

	# Download otherwise

	# Path to downloaded archive
	sources_filename = download_url.split('/')[-1]
	source_archive_filepath = os.path.join(working_directory_path, sources_filename)

	# Download & extract
	download_if_doesnt_exist(download_url, source_archive_filepath)
	unzip_inplace(source_archive_filepath)

	# Rename version-dependent folder name to simpler one
	# In case other builder will need to use this folder
	if inner_dir_name is not None:
		shutil.move(os.path.join(working_directory_path, inner_dir_name), result_folder_path)

	download_and_extract.cache.append(download_data)