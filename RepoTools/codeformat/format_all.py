#!/usr/bin/env python

import fnmatch
import os
import subprocess
import errno
import sys

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("-t", "--teamcity-notify", help="print list of non-formatted files in Teamcity format", action="store_true")
args = parser.parse_args()
formatOK = True


def check_format(file, formatOK):
	proc = subprocess.Popen([cwd+'/'+execName, '-output-replacements-xml', '--style=file', file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	stdout, stderr = proc.communicate()
	if stdout.find("<replacement ") > -1:
		formatOK = False
		errorMsg = "##teamcity[message text=\'" + "%s not formatted" % file + "\' errorDetails=\'\' status=\'" + "ERROR" + "\']\n"
		print errorMsg
	return formatOK

def format(file):
	proc = subprocess.Popen([cwd+'/'+execName, '-i', '--style=file', file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

cwd = os.getcwd()
if os.name == 'nt':
	execName = 'clang-format.exe'
else:
	execName = 'clang-format'
sources = ['../../Sources/Internal', '../../Projects', '../../Tools']
for source in sources:
	for root, dirnames, filenames in os.walk(source):
		for ext in ['cpp', 'h', 'c', 'mm']:
			for filename in fnmatch.filter(filenames, '*.'+ext):
				file = os.path.join(root, filename)
				if args.teamcity_notify:
					formatOK = check_format(file, formatOK)
				else:
					format(file)
					
if args.teamcity_notify:
	if formatOK:
		print "##teamcity[message text=\'" + "format OK" + "\' errorDetails=\'\' status=\'" + "NORMAL" + "\']\n"
	else:
		exit("not all files formatted")