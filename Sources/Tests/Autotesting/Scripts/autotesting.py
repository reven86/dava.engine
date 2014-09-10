#!/usr/bin/env python2.6
#
#  autotesting.py
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
import random;
import subprocess;

arguments = sys.argv[1:]

if len(arguments) < 5 or len(arguments) > 6:
    print 'Usage: ./autotesting_init.py PlatformName ProjectFolder TargetName ConfigurationName Device [MasterPlatform]'
    exit(1)

print "*** DAVA Starting autotesting"

print "platform.system: " + platform.system()
print sys.argv

# Parse CLI args
platformName = arguments[0]
projectFolder = arguments[1]
targetName = arguments[2]
configurationName = arguments[3]
device = arguments[4]

# Prepare working path's
currentDir = os.getcwd(); 
frameworkDir =  os.path.realpath(currentDir + "/../../../")
projectDir = os.path.realpath(currentDir + "/../../../../" + projectFolder)
print "Framework directory:" + frameworkDir
print "Project directory:" + projectDir

autotestingSrcFolder = os.path.realpath(projectDir + "/Autotesting")
autotestingDestFolder = os.path.realpath(projectDir + "/Data/Autotesting")	

executableName = ""
executableBuildPath = ""
executableRunPath = ""
testsFolder = os.path.realpath(projectDir + "/Data/Autotesting/Tests")

testsFolderiOS = os.path.realpath(projectDir + "/build/" + configurationName + "-iphoneos/" + targetName + ".app/Data/Autotesting/Tests")
testsFolderMacOS = os.path.realpath(projectDir + "/build/" + configurationName + "/" + targetName + ".app/Contents/Resources/Data/Autotesting/Tests")

UID = ""
HANDLE = ""

# Prepare application to launch
if (platform.system() == "Windows"):
    executableName = targetName + ".exe"
    print "executableName: " +executableName
    
    if(platformName == "Windows"):
        #TODO: Windows
        print "prepare to run " + executableName + " on Windows"
        
        executableBuildPath = os.path.realpath(projectDir + "/" + configurationName + "/" + executableName)
        executableRunPath = os.path.realpath(projectDir + "/" + executableName)
        
        if os.path.exists(executableRunPath):    
            print "delete " + executableRunPath
            os.remove(executableRunPath)
        print "copy " + executableBuildPath + " to " + executableRunPath
        shutil.copy(executableBuildPath, executableRunPath)
    
        testsFolder = os.path.realpath(projectDir + "/Data/Autotesting/Tests")
    
    else:
        print "Error: wrong OS " + platformName
elif (platform.system() == "Darwin"):
    executableName = targetName + ".app"
    print "executableName: " +executableName
    
    if (platformName == "iOS"):

        print "prepare to run " + executableName + " on iOS"
        # copy executable to Autotesting
        executableBuildPath = os.path.realpath(projectDir + "/build/" + configurationName + "-iphoneos/" + executableName)
        executableRunPath = os.path.realpath(autotestingSrcFolder + "/" + executableName)
        if os.path.exists(executableRunPath):    
            print "delete " + executableRunPath
            shutil.rmtree(executableRunPath)
        print "copy " + executableBuildPath + " to " + executableRunPath
        shutil.copytree(executableBuildPath, executableRunPath)
        
        # resign and deploy
        print "cd " + autotestingSrcFolder
        os.chdir(autotestingSrcFolder)
        
        #sh floatsign.sh $2.app $3 $2.ipa
        ipaName = targetName + ".ipa"
        
        params = ["sh", "./packipa.sh", executableName, ipaName]
        print "create " + ipaName
        
        print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params)
    
        # Delete and installl app to device
        print "get device id"
        params = "~/AIRSDK_Compiler/bin/adt -devices -platform iOS | grep '" + device + "' | awk '{print $3}'"
        print "subprocess.call " + params
        UID = subprocess.check_output(params, shell=True).split('\n')[0]
        
        params = "~/AIRSDK_Compiler/bin/adt -devices -platform iOS | grep '" + device + "' | awk '{print $1}'"
        print "subprocess.call " + params
        HANDLE = subprocess.check_output(params, shell=True).split('\n')[0]
        
        print "Device id " + UID + ", device handle " + HANDLE


		# Remove old App from device
        print "remove "+ executableName +" from device"
        #params = ["~/AIRSDK_Compiler/bin/adt", "-uninstallApp", "-platform", platformName, "-appid", "com.yourcompany." + targetName]
        params = "~/AIRSDK_Compiler/bin/adt -uninstallApp -platform iOS -device " + HANDLE + " -appid com.davainc."+targetName
        print "subprocess.call " + params
        #print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params, shell=True)
		
        # Install app to device
        print "remove "+ executableName +" from device"
        #params = ["~/AIRSDK_Compiler/bin/adt", "-uninstallApp", "-platform", platformName, "-appid", "com.yourcompany." + targetName]
        params = "~/AIRSDK_Compiler/bin/adt -installApp -device " + HANDLE + " -platform iOS -package " + ipaName
        print "subprocess.call " + params
        #print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
        subprocess.call(params, shell=True)

        
        testsFolder = testsFolderiOS

    elif (platformName == "MacOS"):
        #TODO: MacOS
        print "prepare to run " + executableName + " on MacOS"
        executableBuildPath = os.path.realpath(projectDir + "/build/" + configurationName + "/" + executableName + "/Contents/MacOS/" + targetName)
        executableRunPath = executableBuildPath

        testsFolder = testsFolderMacOS
    else:
        print "Error: wrong OS " + platformName

else:
    print "Error: unsupported platform.system: " + platform.system()    

os.chdir(projectDir)

# the following code is specially for configuration that runs iOS and MacOS autotests on single agent
if 6 == len(arguments):
    
    masterPlatform = arguments[5]
    print "get tests count from master " + masterPlatform

    testFilesInFolder = os.listdir(testsFolder)
    testFilesInFolderCount = len(testFilesInFolder)
    
    testsCount = testFilesInFolderCount
    
    if (platform.system() == "Darwin"):
        if (masterPlatform == "iOS"):
            print "get tests count from " + testsFolderiOS
            testsCount = len(os.listdir(testsFolderiOS))
        elif (masterPlatform == "MacOS"):
            print "get tests count from " + testsFolderMacOS
            testsCount = len(os.listdir(testsFolderMacOS))

    print "testsCount=" + str(testsCount)

    if testsCount <= testFilesInFolderCount:
        testFiles = testFilesInFolder[0:testsCount]
    else:
        testFiles = []
        
        if testFilesInFolderCount > 0:
            testsCountLeft = testsCount
            while testsCountLeft > 0:
                if testsCountLeft <= testFilesInFolderCount:
                    testFiles.extend(testFilesInFolder[0:testsCountLeft])
                else:
                    testFiles.extend(testFilesInFolder)
                testsCountLeft -= testFilesInFolderCount
    print "testFiles=" + "[%s]" % ", ".join(map(str, testFiles))
else:
    testFiles = os.listdir(testsFolder)

# Launch tests
for testFile in testFiles:
    print "current test file is: " + testFile
    
    if (platform.system() == "Windows"):
        if(platformName == "Windows"):
            #TODO: Windows
            print "run " + executableName + " on Windows"
            #os.system("start /WAIT " + executableName)
            params = [executableName]
            os.spawnv(os.P_WAIT, executableName, params)
        else:
            print "Error: wrong OS " + platformName
    elif (platform.system() == "Darwin"):
        if (platformName == "iOS"):
            #TODO: iOS
            print "run " + executableName + " on iOS"
            
            os.chdir(autotestingSrcFolder)
            
            #instruments -w $4 -t /Developer/Platforms/iPhoneOS.platform/Developer/Library/Instruments/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate "$2" -e UIASCRIPT testRun.js

            #params = ["sh", "./runOnDevice.sh", targetName, device]
            params = ["instruments", "-w", UID, "-t", os.environ["PATH_TO_AUTO_TEMPL"] + "/Automation.tracetemplate", targetName, "-e", "UIASCRIPT", "testRun.js"]
            print "subprocess.call " + "[%s]" % ", ".join(map(str, params))
            subprocess.call(params)
        
        elif (platformName == "MacOS"):
            #TODO: MacOS
            print "run " + executableRunPath + " on MacOS"
            #os.system("open -W " + executableRunPath)

            params = [executableRunPath]
            os.spawnv(os.P_WAIT, executableRunPath, params)
        else:
            print "Error: wrong OS " + platformName
    else:
        print "Error: unsupported platform.system: " + platform.system()

print "TODO: report"

print "TODO: cleanup"    
if os.path.exists(autotestingDestFolder):    
    print "Delete " + autotestingDestFolder
#shutil.rmtree(autotestingDestFolder)

if (platformName == "iOS"):
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
#shutil.rmtree(executableRunPath)
elif (platformName == "Windows"):
    if os.path.exists(executableRunPath):    
        print "delete " + executableRunPath
        os.remove(executableRunPath)
   
print "*** DAVA Finished autotesting"