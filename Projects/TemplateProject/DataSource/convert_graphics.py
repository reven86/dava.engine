#!/usr/bin/python
#
#  convert_graphics.py
#  DAVA SDK
#
#  Created by Vitaliy Borodovsky on 10/27/08.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os
import sys
import os.path
import pickle
import zlib
import string
import sys
import platform
import subprocess

def main_convert_graphics(): # it's function for external usage
	# *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************
	framework_path = { "Darwin": "./../../dava.framework", "Windows": "./../../dava.framework", "Microsoft": "./../../dava.framework" }
	# *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************

	currentDir = os.getcwd(); 
	dataDir =  os.path.realpath(currentDir + "/../Data/")
	print "*** DAVA SDK Launching command line packer - data directory:" + dataDir

	subprocess.call([sys.executable, 'touch_Data.py'])
	
	params = filter(lambda x: x[0] != '-', sys.argv[1:])
	#print params

	flags = sys.argv[1:]
	#print flags

	#gfxDirs = filter(lambda x: x[0:3] == "Gfx", os.listdir(currentDir))
	gfxDirs = ["Gfx"]
	# print gfxDirs

	pvrTexToolPathname = framework_path[platform.system()] + "/Tools/Bin/"
	if (framework_path[platform.system()] != ""):
		os.chdir(framework_path[platform.system()] + "/Tools/Bin/")
		for dir in gfxDirs:
			params = ["./ResourcePacker", os.path.realpath(currentDir + "/" + dir)] + [pvrTexToolPathname] + flags
			print "[%s]" % ", ".join(map(str, params))
			os.spawnv(os.P_WAIT, "./ResourcePacker", params)
	else:
		print "Framework path not defined, please define it in dava_framework_path.py"

	# come back
	os.chdir(currentDir)
		
if __name__ == '__main__':
	main_convert_graphics()