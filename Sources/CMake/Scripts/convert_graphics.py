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
import shutil
import time;


def delete_folder(folder_path):
    if os.path.exists( folder_path ):
        print 'delete folder ', folder_path
        shutil.rmtree( folder_path )

arguments = sys.argv[1:]

if len(arguments) != 0:
    currentDir = arguments[0];
    framework_path = { "Darwin": arguments[1], "Windows": arguments[1] }
    os.chdir( currentDir )
else:
    currentDir = os.getcwd(); 
    framework_path = { "Darwin": "./../../../", "Windows": "./../../../" }

delete_folder( os.path.join(currentDir, "$process")  )
delete_folder( os.path.join(currentDir, "../Data/Gfx")  )

# *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************

dataDir =  os.path.realpath(currentDir + "/../Data/")
print "*** DAVA SDK Launching command line packer - data directory:" + dataDir


params = filter(lambda x: x[0] != '-', sys.argv[1:]);
#print params

flags = sys.argv[1:];
#print flags

gfxDirs = filter(lambda x: x[0:3] == "Gfx", os.listdir(currentDir));
#print gfxDirs

startTime = time.time()

pvrTexToolPathname = framework_path[platform.system()] + "/Tools/Bin/"
if (framework_path[platform.system()] != ""):
    os.chdir(framework_path[platform.system()] + "/Tools/Bin/");
    for dir in gfxDirs:
        params = ["./ResourcePacker", os.path.realpath(currentDir + "/" + dir)] + [pvrTexToolPathname] + flags;
        os.spawnv(os.P_WAIT, "./ResourcePacker", params);
else:
	print "Framework path not defined, please define it in dava_framework_path.py"

deltaTime = time.time() - startTime;
print "Operation  time", deltaTime; 

