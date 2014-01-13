#!/usr/bin/python
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

def main_convert_graphics(): # it's function for external usage
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************
    framework_path = { "Darwin": "./../../..", "Windows": "./../../..", "Microsoft": "./../../.." }
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************

    currentDir = os.getcwd(); 
    output =  os.path.realpath(currentDir + "/Data/")
    print "*** DAVA SDK Launching command line packer - data directory:" + output

    params = filter(lambda x: x[0] != '-', sys.argv[1:])
    #print params

    flags = sys.argv[1:]
    #print flags

    input = os.path.realpath(currentDir + "/TestData/")
    #print gfxDirs

    pvrTexToolPathname = framework_path[platform.system()] + "/Tools/Bin/"
    if (framework_path[platform.system()] != ""):
        os.chdir(pvrTexToolPathname)
        params = ["./ResourcePacker", input] + [pvrTexToolPathname] + flags
        os.spawnv(os.P_WAIT, "./ResourcePacker", params)
    else:
        print "Framework path not defined, please define it in dava_framework_path.py"

    # come back
    os.chdir(currentDir)
        
if __name__ == '__main__':
    main_convert_graphics()