#!/usr/bin/env python
#
#  copy_lka.py
#  DAVA SDK
#
#  Created by Yury Danilov on 6/14/12.
#  Copyright (c) 2012 DAVA Consulting, LLC. All rights reserved.
#



import os
import sys
import os.path
import string
import sys
import platform
import shutil
import subprocess

currentDir = os.getcwd(); 

def ignored_svn_files(adir,filenames):
    return [filename for filename in filenames if filename.endswith(".svn")]

def copy_replace_folder(srcFolder, destFolder):
    if os.path.exists(destFolder):   
        print "shutil.rmtree " + destFolder 
        shutil.rmtree(destFolder)
    print "copy " + srcFolder + " to " + destFolder
    shutil.copytree(srcFolder, destFolder, ignore=ignored_svn_files)
    
dataSubfoldersToCopy = ["Materials/", "Shaders/"]
resourceEditorPlatformPath = { "Darwin": "./ResourceEditor.app/Contents/Resources", "Windows": "./ResourceEditor", "Microsoft": "./ResourceEditor" }
resourceEditorPath = resourceEditorPlatformPath[platform.system()]

for dataSubfolder in dataSubfoldersToCopy:
    #copy to project
    dataSubDir =  os.path.realpath(currentDir + "/../Data/" + dataSubfolder)
    frameworkSubDir =  os.path.realpath(currentDir + "./../ResourceEditor/dava.framework/Tools/" + resourceEditorPath + "/Data/" + dataSubfolder)
    copy_replace_folder(frameworkSubDir, dataSubDir)