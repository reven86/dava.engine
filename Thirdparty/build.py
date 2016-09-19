#!/usr/bin/env python2.7

import imp
import os
import sys
import traceback

# List of invoked builders to avoid multiple invokes
invoked_builders = []

def build_library(name):
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

	for dependency in builder.get_internal_dependencies_for_platform(current_platform):
		print 'Building declared dependency: {}'.format(dependency)
		if build_library(dependency) == False:
			print 'Skipping library {} since its dependency ({}) couldn\' be built'.format(name, dependency)
			return False;

	# Build

	output_path = os.path.join('output', name)
	if not os.path.exists(output_path):
		os.makedirs(output_path)

	os.chdir(name)

	try:
		result = builder.build(os.path.join('..', output_path), '../..')
	except Exception:
		print 'Couldn\'t build library {}. Exception details:'.format(name)
		traceback.print_exc()
		result = False

	os.chdir('..')

	return result

if __name__ == "__main__":
	sys.path.append('Private')

	if (len(sys.argv) > 1):
		for library_name in sys.argv[1:]:
			build_library(library_name)
	else:
		for folder_name in os.listdir(os.getcwd()):
			if os.access(os.path.join(folder_name, 'build.py'), os.F_OK):
				build_library(folder_name)