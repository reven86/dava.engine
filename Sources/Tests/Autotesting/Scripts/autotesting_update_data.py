#!/usr/bin/python
#
#  autotesting_update_data.py
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

arguments = sys.argv[1:]

if len(arguments) < 2 or 3 < len(arguments):
    print 'Usage: ./autotesting_update_data.py [PlatformName] [ProjectName] [TestsGroupName]'
    exit(1)

print "*** DAVA Initializing autotesting (without copying Config.h)"
platformName = arguments[0]
projectName = arguments[1]
testsGroupName = "default"

testsSrcFolder = "/Tests"
if 3 == len(arguments):
    testsGroupName = arguments[2]
    testsSrcFolder = testsSrcFolder + "/" + testsGroupName

print "platform.system: " + platform.system()

currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + projectName)
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")
    
scripts = ["/generate_id.py", "/copy_tests.py"]

if (platform.system() == "Darwin"):
    if (platformName == "iOS"):
        scripts.append("/runOnDevice.sh")
        scripts.append("/floatsign.sh")
        scripts.append("/packipa.sh")
        scripts.append("/transporter_chief.rb")
        scripts.append("/testRun.js")

autotestingReportsFolder = os.path.realpath(autotestingSrcFolder + "/Reports")      

if os.path.exists(autotestingReportsFolder):   
    print "remove previous report for " + platformName       
    autotestingReportPath = os.path.realpath(autotestingReportsFolder + "/report_" + platformName + ".html") 
    if os.path.exists(autotestingReportPath): 
        os.remove(autotestingReportPath)
else:
    os.mkdir(autotestingReportsFolder)    
    
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

luaScriptName = "/autotesting_api.lua"
luaScriptSrcPath = os.path.realpath(currentDir + luaScriptName)
luaScriptDestPath = os.path.realpath(luaScriptDestFolder + luaScriptName)
if os.path.exists(luaScriptDestPath):
    print "delete " + luaScriptDestPath
    os.remove(luaScriptDestPath)
print "copy " + luaScriptSrcPath + " to " + luaScriptDestPath
shutil.copy(luaScriptSrcPath, luaScriptDestPath)

os.chdir(autotestingSrcFolder)

params = ["python", "./copy_tests.py", testsSrcFolder, autotestingDestFolder]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)

params = ["python", "./generate_id.py", projectName, autotestingDestFolder, testsGroupName]
print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
subprocess.call(params)

os.chdir(currentDir)
   
print "*** DAVA Initialized autotesting (without copying Config.h)"