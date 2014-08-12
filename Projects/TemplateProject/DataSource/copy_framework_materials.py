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
    
for dataSubfolder in dataSubfoldersToCopy:
    #copy to project
    dataSubDir =  os.path.realpath(currentDir + "/../Data/" + dataSubfolder)
    frameworkSubDir =  os.path.realpath(currentDir + "./../../dava.framework/Tools/ResourceEditor/Data/" + dataSubfolder)
    copy_replace_folder(frameworkSubDir, dataSubDir)

    #also copy to TankEditor, TankViewer, EffectViewer
    dataSubDirTE = os.path.realpath(currentDir + "/../Tools/TankEditor/Data/" + dataSubfolder)
    dataSubDirTV = os.path.realpath(currentDir + "/../Tools/TankViewer/Data/" + dataSubfolder)
    dataSubDirEV = os.path.realpath(currentDir + "/../Tools/EffectViewer/Data/" + dataSubfolder)
    dataSubDirCMC = os.path.realpath(currentDir + "/../Tools/CollisionMeshConverter/Data/" + dataSubfolder)
    dataSubDirTC = os.path.realpath(currentDir + "/../Tools/TerrainConverter/Data/" + dataSubfolder)
    copy_replace_folder(frameworkSubDir, dataSubDirTE)
    copy_replace_folder(frameworkSubDir, dataSubDirTV)
    copy_replace_folder(frameworkSubDir, dataSubDirEV)
    copy_replace_folder(frameworkSubDir, dataSubDirCMC)
    copy_replace_folder(frameworkSubDir, dataSubDirTC)