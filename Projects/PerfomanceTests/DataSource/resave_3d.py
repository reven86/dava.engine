#!/usr/bin/env python
#
#  convert_3d.py
#  DAVA SDK
#
#  Created by Yury Danilov on 6/14/12.
#  Copyright (c) 2012 DAVA Consulting, LLC. All rights reserved.
#

import os
import sys
import os.path
import pickle
import zlib
import string
import sys
import platform
import shutil

class ConsoleWriter :
  def __init__(self, *writers) :
    self.writers = writers

  def write(self, text) :
    for w in self.writers :
      w.write(text) # help broken consoles

stdout_f = open('resave_3d.log', 'w')
sys.stdout = ConsoleWriter(sys.stdout, stdout_f)

def resave_folder(pathToDataSource, relativePathToFolder, executable):
    print "resave_folder " + relativePathToFolder
    realPathToFolder = os.path.realpath(pathToDataSource + "/" + relativePathToFolder)
    folderItems = os.listdir(realPathToFolder)
    for item in folderItems:
        relativePathToItem = relativePathToFolder + "/" + item
        realPathToItem = os.path.realpath(realPathToFolder + "/" + item)
        if item.endswith(".sc2"):
            if relativePathToItem.find(" ") > 0:
                print "ERROR: space in file path: " + relativePathToItem
            print "processfile " + relativePathToItem
            params = [executable, '-scenesaver', '-resave', '-indir', pathToDataSource, '-processfile', relativePathToItem, '-forceclose']
            os.spawnv(os.P_WAIT, executable, params)
        elif os.path.isdir(realPathToItem):
            resave_folder(pathToDataSource, relativePathToItem, executable)

def resave_folders(pathToDataSource, executable):
    realPathToFolder = os.path.realpath(pathToDataSource)
    folderItems = os.listdir(realPathToFolder)
    for item in folderItems:
        realPathToItem = os.path.realpath(realPathToFolder + "/" + item)
        if os.path.isdir(realPathToItem):
            resave_folder(pathToDataSource, item, executable)
            
def main_resave_3d(): # it's function for external usage

    currentDir = os.getcwd(); 
    
    dataSource3dDir = os.path.realpath(currentDir + "/3d/")

    index = dataSource3dDir.find(':')
    if ( index != -1):
        dataSource3dDir = dataSource3dDir[index + 1:]

    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************
    executables = { "Darwin": "./ResourceEditor.app/Contents/MacOS/ResourceEditor", "Windows": "./ResourceEditor", "Microsoft": "./ResourceEditor" }
                                 
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************

    #executable = './ResourceEditor.app/Contents/MacOS/TemplateProjectMacOS'
    #executable = "./ResourceEditor"
    #print "DEBUG: current dir: [" + os.getcwd() + "]"
    os.chdir("../Tools/ResEditor/dava.framework/Tools/ResourceEditor/");

    executable = executables[platform.system()];
    print "ex: " + executable;

    # export files
    resave_folders(dataSource3dDir, executable)
    
    # come back
    os.chdir(currentDir)

if __name__ == '__main__':  
    main_resave_3d()

stdout_f.close()