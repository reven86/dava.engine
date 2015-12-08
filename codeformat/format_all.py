#!/usr/bin/env python

import fnmatch
import os
import subprocess
import errno

cwd = os.getcwd()
sources = ['../Sources/Internal', '../Projects', '../Tools']
for source in sources:
	for root, dirnames, filenames in os.walk(source):
		for ext in ['cpp', 'h', 'c', 'mm']:
			for filename in fnmatch.filter(filenames, '*.'+ext):
				try:
					proc = subprocess.Popen([cwd+'/clang-format', '-i', '--style=file', os.path.join(root, filename)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
				except OSError as e:
					if e.errno == errno.ENOENT:
						exit('cannot find executable clang-format')
					else:
						raise
