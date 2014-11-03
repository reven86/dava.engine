#!/usr/bin/python
#
#  copy_tests.py
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

print "copy_tests" 

arguments = sys.argv[1:]


def ignored_svn_files(adir,filenames):
    return [filename for filename in filenames if filename.endswith(".svn")]

def copy_replace_folder(srcFolder, destFolder):
    if os.path.exists(destFolder):   
        print "shutil.rmtree " + destFolder 
        shutil.rmtree(destFolder)
    print "copy " + srcFolder + " to " + destFolder
    shutil.copytree(srcFolder, destFolder, ignore=ignored_svn_files)

projectFolder = os.getcwd()

autotestingActionsSrcFolder = os.path.realpath(projectFolder + "/Autotesting/Actions")
autotestingActionsDestFolder = os.path.realpath(projectFolder + "/Data/Autotesting/Actions")
copy_replace_folder(autotestingActionsSrcFolder, autotestingActionsDestFolder)

autotestingTestsSrcFolder = os.path.realpath(projectFolder + "/Autotesting/Tests")
autotestingTestsDestFolder = os.path.realpath(projectFolder + "/Data/Autotesting/Tests")
copy_replace_folder(autotestingTestsSrcFolder, autotestingTestsDestFolder)

autotestingStringsSrcFolder = os.path.realpath(projectFolder + "/Autotesting/Strings")
autotestingStringsDestFolder = os.path.realpath(projectFolder + "/Data/Autotesting/Strings")
copy_replace_folder(autotestingStringsSrcFolder, autotestingStringsDestFolder)


print "copy_tests done" 