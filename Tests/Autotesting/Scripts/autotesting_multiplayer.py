#!/usr/bin/env python2.6
#
#  autotesting_multiplayer.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os
import sys
import os.path
import string
import platform
import shutil
import random
import subprocess
import time

arguments = sys.argv[1:]

if len(arguments) != 7:
    print 'Usage: ./autotesting_multiplayer.py ProjectFolder ConfigurationName MasterPlatformName MasterTargetName HelperPlatformName HelperTargetName Device'
    exit(1)


print "*** DAVA Starting multiplayer autotesting"

print "platform.system: " + platform.system()
print sys.argv

projectFolder = arguments[0]
configurationName = arguments[1]

masterPlatformName = arguments[2]
masterTargetName = arguments[3]

helperPlatformName = arguments[4]
helperTargetName = arguments[5]

device = arguments[6]


masterParams = ["python", "./autotesting.py", masterPlatformName, projectFolder, masterTargetName, configurationName, device]
helperParams = ["python", "./autotesting.py", helperPlatformName, projectFolder, helperTargetName, configurationName, device, masterPlatformName]

allParams = [masterParams, helperParams]
running_procs = []

for params in allParams:
    #running_procs.append(subprocess.Popen(params, stdout=subprocess.PIPE, stderr=subprocess.PIPE))
    print "subprocess.Popen " + "[%s]" % ", ".join(map(str, params))
    running_procs.append(subprocess.Popen(params))
    time.sleep(1.0)

while running_procs:
    for proc in running_procs:
        retcode = proc.poll()
        if retcode is not None: # Process finished.
            running_procs.remove(proc)
            break
    else: # No process is done, wait a bit and check again.
        time.sleep(.1)
        continue

print "*** DAVA Finished multiplayer autotesting"