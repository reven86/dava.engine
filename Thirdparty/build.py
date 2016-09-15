#!/usr/bin/env python2.7
import os
import sys
import subprocess

def build_library(folder_name):
	if os.access(os.path.join(folder_name, "build.py"), os.F_OK):
		sp = subprocess.Popen(["python", "build.py"], cwd=folder_name)
		sp.wait()

if len(sys.argv) > 1:
	build_library(sys.argv[1])
else:
	for folder in os.listdir(os.getcwd()):
		build_library(folder)

