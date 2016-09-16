#!/usr/bin/env python2.7
import urllib2
import zipfile
import os
import subprocess
import glob
import shutil

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

	print "Download OK"

def download_if_doesnt_exist(url, file_name):
	if not os.path.exists(file_name):
		download(url, file_name)

def unzip_inplace(path):
	print "Unzipping %s ..." % (path)
	zip_ref = zipfile.ZipFile(path, 'r')
	zip_ref.extractall(os.path.dirname(path))
	zip_ref.close()
	print "Unzipping OK"

def apply_patch(patch):
	print "Applying patch %s" % patch
	proc = subprocess.Popen(["git", "apply", "--ignore-whitespace", patch], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	proc.communicate()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise 
	else:
		print "Applying patch OK"

def build_vs(project, configuration, platform='Win32', target = None):
	print "Building %s for %s ..." % (project, configuration)
	args = ["c:/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe", project, "/p:Configuration="+configuration]
	if (not target is None):
		args.append('/target:' + target)
	proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	proc.communicate()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise 
	else:
		print "Build OK"

def build_xcode_target(project, target, configuration):
	print "Building %s for %s ..." % (project, configuration)
	proc = subprocess.Popen(["xcodebuild", "-project", project, "-target", target, "-configuration", configuration, "build"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	proc.communicate()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise
	else:
		print "Build OK"

def build_xcode_alltargets(project, configuration):
	print "Building %s for %s ..." % (project, configuration)
	proc = subprocess.Popen(["xcodebuild", "-project", project, "-alltargets", "-configuration", configuration, "build"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	proc.communicate()
	if proc.returncode != 0:
		print "Failed with return code %s" % proc.returncode
		raise
	else:
		print "Build OK"

def copy_files(from_dir, to_dir, wildcard):
	print "Copying %s from %s to %s" % (wildcard, from_dir, to_dir)
	for file in glob.glob(from_dir+"/"+wildcard):
		shutil.copy(file, to_dir)
	print "Copying OK"

def clean_copy_includes(from_dir, to_dir):
	print "Copying includes from %s to %s" % (from_dir, to_dir)
	shutil.rmtree(to_dir)
	shutil.copytree(from_dir, to_dir)
	print "Copying includes OK"

def clear_files(dir, wildcard):
	print "Deleting %s in %s" % (wildcard, dir)
	map(os.remove, glob.glob(wildcard))
	print "Deleting OK"

def cmake_generate(output_folder_path, src_folder_path, cmake_generator, cmake_additional_args = []):
	if not os.path.exists(output_folder_path):
		os.makedirs(output_folder_path)

	cmd = ['cmake', '-G', cmake_generator, src_folder_path]
	cmd.extend(cmake_additional_args)
	sp = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=output_folder_path)
	for line in sp.stdout:
		print line
	sp.wait()

def cmake_generate_build_vs(output_folder_path, src_folder_path, cmake_generator, sln_name, target, configuration, cmake_additional_args = []):
	cmake_generate(output_folder_path, src_folder_path, cmake_generator, cmake_additional_args)
	build_vs(os.path.join(output_folder_path, sln_name), 'Debug', configuration, target)
	build_vs(os.path.join(output_folder_path, sln_name), 'Release', configuration, target)

def build_android_ndk(project_path, output_path, debug):
	cmd = ['ndk-build', 'NDK_OUT=' + output_path]
	if debug:
		cmd.append('NDK_DEBUG=1')
	sp = subprocess.Popen(cmd, stdout=subprocess.PIPE, cwd=project_path, shell=True)
	for line in sp.stdout:
		print line
	sp.wait()