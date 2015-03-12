#!/usr/bin/env python2.5
#
#  convert_graphics.py
#  DAVA SDK
#
#  Created by Vitaliy Borodovsky on 10/27/08.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import pickle;
import zlib;
import string;
import sys;
import platform;

arguments = sys.argv[1:]

if len(arguments) == 1:
    currentDir = arguments[0];
    os.chdir( currentDir )
else:
    currentDir = os.getcwd(); 


# *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************
framework_path = { "Darwin": "./../../../", "Windows": "./../../../" }
# *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************


dataDir =  os.path.realpath(currentDir + "/../Data/")
print "*** DAVA SDK Launching command line packer - data directory:" + dataDir


params = filter(lambda x: x[0] != '-', sys.argv[1:]);
#print params

flags = sys.argv[1:];
#print flags

gfxDirs = filter(lambda x: x[0:3] == "Gfx", os.listdir(currentDir));
#print gfxDirs

pvrTexToolPathname = framework_path[platform.system()] + "/Tools/Bin/"
if (framework_path[platform.system()] != ""):
    os.chdir(framework_path[platform.system()] + "/Tools/Bin/");
    for dir in gfxDirs:
        params = ["./ResourcePacker", os.path.realpath(currentDir + "/" + dir)] + [pvrTexToolPathname] + flags;
        os.spawnv(os.P_WAIT, "./ResourcePacker", params);
else:
	print "Framework path not defined, please define it in dava_framework_path.py"
