#!/usr/bin/env python2.7

import argparse
import imp
import os
import sys
import traceback

sys.path.append('Private')
import build_utils

# Imported builders dict to avoid multiple imports
imported_builders = {}

def import_library_builder_module(name):
	if not name in imported_builders:
		builder_file_path = os.path.join(name, 'build.py')
		imported_builders[name] = imp.load_source('dava_builder_' + name, builder_file_path)

	return imported_builders[name]

# List of invoked builders to avoid multiple invokes
invoked_builders = []

def build_library(name, targets, skip_dependencies):
	# Check if we already invoked builder for this library
	# (it could be declared as a dependency for another library)
	# Return None in this case since we didn't remember the result

	if name in invoked_builders:
		return None

	print 'Started building library: {}...'.format(name)
	invoked_builders.append(name)

	# Load builder module

	builder_file_path = os.path.join(name, 'build.py')
	try:
		builder = imp.load_source('dava_builder_' + name, builder_file_path)
	except Exception as e:
		print 'Couldn\'t load builder file at path {}. Exception details:'.format(builder_file_path)
		traceback.print_exc()
		return False

	# Check if it should be built on current platform
	
	current_platform = sys.platform
	library_platforms = builder.get_supported_build_platforms()
	if not current_platform in library_platforms:
		print 'Skipping library {} since it can\'t or shouldn\'t be built on current platform ({})'.format(name, current_platform)
		return False

	# Check dependencies

	if not skip_dependencies:
		dependencies = get_dependencies_for_library(builder, targets)

		for dependency in dependencies:
			print 'Building declared dependency: {}'.format(dependency)
			if build_library(dependency, targets, skip_dependencies) == False:
				print 'Skipping library {} since its dependency ({}) couldn\' be built'.format(name, dependency)
				return False;

	# Build

	library_working_dir = os.path.abspath(os.path.join('output', name))
	project_root_path = os.path.abspath('..')
	if not os.path.exists(library_working_dir):
		os.makedirs(library_working_dir)

	os.chdir(name)

	try:
		for target in targets:
			result = builder.build_for_target(target, library_working_dir, project_root_path)
	except Exception:
		print 'Couldn\'t build library {}. Exception details:'.format(name)
		traceback.print_exc()
		result = False

	os.chdir('..')

	return result

def get_all_libraries():
	return [folder_name for folder_name in os.listdir(os.getcwd()) if os.access(os.path.join(folder_name, 'build.py'), os.F_OK)]

def get_dependencies_for_library(builder_module, targets):
	dependencies = []
	for target in targets:
		target_deps = builder_module.get_dependencies_for_target(target)
		for target_dep in target_deps:
			if not target_dep in dependencies:
				dependencies.append(target_dep)

	return dependencies

def print_info(library, targets):
	builder = import_library_builder_module(library)

	download_url = builder.get_download_url()
	supported_targets = { 'win32' : builder.get_supported_targets_for_build_platform('win32'), 'darwin' : builder.get_supported_targets_for_build_platform('darwin') }
	supported_build_platforms = builder.get_supported_build_platforms()
	dependencies = get_dependencies_for_library(import_library_builder_module(library), targets)
	print 'Library: {}\nDownload url: {}\nSupported targets: {}\nSupported build platforms: {}\nDependencies: {}\n'.format(library, download_url, supported_targets, supported_build_platforms, dependencies)

if __name__ == "__main__":
	if sys.platform == 'win32':
		all_targets = ['win32', 'win10', 'android']
	else:
		all_targets = ['ios', 'macos', 'android']

	# Setup and parse arguments

	parser = argparse.ArgumentParser()
	parser.add_argument('libs', nargs='*', help='list of libraries to build (build all if none specified)')
	parser.add_argument('-t', '--target', default='all', nargs='?', choices=all_targets, help='targets to build for (build for all targets if not specified)')
	parser.add_argument('-sd', '--skip-dependencies', action='store_true', help='build without first building dependencies')
	parser.add_argument('-i', '--info', action='store_true', help='only show information about selected libraries (all if none was specified)')
	parser.add_argument('-v', '--verbose', action='store_true', help='print additional logs')
	args = parser.parse_args()

	# Prepare and verify data

	targets_to_process = all_targets if args.target == 'all' else [ args.target ]
	if 'android' in targets_to_process:
		ndk_path = build_utils.get_android_ndk_folder_path('..')
		if ndk_path is None:
			print 'Android target is specified, but Android NDK folder couldn\'t be found (required for CMake). Did you forget to put it into DavaConfig.in?\nAborting'
			sys.exit()

	libraries_to_process = list(set(args.libs)) if args.libs != [] else get_all_libraries()
	for lib in libraries_to_process:
		try:
			import_library_builder_module(lib)
		except IOError:
			print 'Couldn\'t import builder module for library named \'{}\', misprint?\nAborting'.format(lib)
			sys.exit()

	# Process

	if args.info:
		for lib in libraries_to_process:
			print_info(lib, targets_to_process)
	else:
		for lib in libraries_to_process:
			build_library(lib, targets_to_process, args.skip_dependencies)