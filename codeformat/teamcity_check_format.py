#!/usr/bin/env python

import fnmatch
import os
import subprocess
import errno

formatOK = True
cwd = os.getcwd()
if os.name == 'nt':
	execName = 'clang-format.exe'
else:
	execName = 'clang-format'
sources = ['../Sources/Internal', '../Projects', '../Tools']
for source in sources:
	for root, dirnames, filenames in os.walk(source):
		for ext in ['cpp', 'h', 'c', 'mm']:
			for filename in fnmatch.filter(filenames, '*.'+ext):
				try:
					proc = subprocess.Popen([cwd+'/'+execName, '-output-replacements-xml', '--style=file', os.path.join(root, filename)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
				except OSError as e:
					if e.errno == errno.ENOENT:
						exit('cannot find executable clang-format')
					else:
						raise
				stdout, stderr = proc.communicate()
				if stdout.find("<replacement ") > -1:
					formatOK = False
					errorMsg = "##teamcity[message text=\'" + "%s not formatted" % os.path.join(root, filename) + "\' errorDetails=\'\' status=\'" + "ERROR" + "\']\n"
					print errorMsg

if formatOK:
	print "##teamcity[message text=\'" + "format OK" + "\' errorDetails=\'\' status=\'" + "NORMAL" + "\']\n"
else:
	exit("not all files formatted")
