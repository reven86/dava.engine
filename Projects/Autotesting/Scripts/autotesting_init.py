#!/usr/bin/python
#
#  autotesting_init.py
#  DAVA SDK
#
#  Created by Dmitry Shpakov on 6/13/12.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os;
import sys;
import os.path;
import string;
import platform;
import shutil;
import subprocess;
import filecmp;

arguments = sys.argv[1:]

if len(arguments) != 2:
    print 'Usage: ./autotesting_init.py PlatformName ProjectFolder'
    exit(1)

def copy_file(srcFolder, destFolder, fileName):
    fileSrcPath = os.path.realpath(srcFolder + "/" + fileName)
    fileDestPath = os.path.realpath(destFolder + "/" + fileName)
    if os.path.exists(fileDestPath):
        print "delete " + fileDestPath
        os.remove(fileDestPath)
    print "copy " + fileSrcPath + " to " + fileDestPath
    shutil.copy(fileSrcPath, fileDestPath)
    
print "*** DAVA Initializing autotesting"
platformName = arguments[0]
projectFolder = arguments[1]

print "platform.system: " + platform.system()

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + projectFolder)
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir



autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
scriptsSrcFolder = os.path.realpath(projectDir + "/Scripts")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")
    
scripts = ["/copy_tests.py"]

if (platform.system() == "Darwin"):
    if (platformName == "iOS"):
        #scripts.append("/runOnDevice.sh")
        #scripts.append("/floatsign.sh")
        scripts.append("/packipa.sh")
        #scripts.append("/transporter_chief.rb")
        scripts.append("/testRun.js")

    
print "copy scripts from " + currentDir + " to " + autotestingSrcFolder
for scriptName in scripts:
    scriptSrcPath = os.path.realpath(currentDir + scriptName)
    scriptDestPath = os.path.realpath(autotestingSrcFolder + scriptName)
    if os.path.exists(scriptDestPath):    
        print "delete " + scriptDestPath
        os.remove(scriptDestPath)
    print "copy " + scriptSrcPath + " to " + scriptDestPath
    shutil.copy(scriptSrcPath, scriptDestPath)

if os.path.exists(autotestingDestFolder):    
    print "Autotesting already exists - delete " + autotestingDestFolder
    shutil.rmtree(autotestingDestFolder)

os.mkdir(autotestingDestFolder)

luaScriptDestFolder = os.path.realpath(autotestingDestFolder + "/Scripts")
os.mkdir(luaScriptDestFolder)

copy_file(currentDir, luaScriptDestFolder, "autotesting_api.lua")
copy_file(currentDir, luaScriptDestFolder, "coxpcall.lua")

copy_file(autotestingSrcFolder, autotestingDestFolder, "dbConfig.yaml")
copy_file(".", autotestingDestFolder, "id.yaml")

os.chdir(projectDir)


params = ["python", "./Autotesting/copy_tests.py"]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)
'''
params = ["python", "./Autotesting/generate_id.py", projectName]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)
'''
os.chdir(currentDir)
   
print "*** DAVA Initialized autotesting"