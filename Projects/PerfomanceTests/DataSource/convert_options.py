#!/usr/bin/python
#
#  convert_options.py

import os
import sys
import os.path
import pickle
import zlib
import string
import sys
import platform
import shutil

def main_convert_graphics(): # it's function for external usage
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************
    pathToOptionsConverter = "../Tools/OptionsConverter"
    executables = { "Darwin": "./TemplateProjectMacOS.app/Contents/MacOS/TemplateProjectMacOS", "Windows": "./OptionsConverter", "Microsoft": "./OptionsConverter" }
    # *************** HERE YOU SHOULD SETUP YOUR OWN PATHS ***************

    currentDir = os.getcwd()
    dataDir =  os.path.realpath(currentDir + "/../Data/")
    print "*** Options Converter - data directory:" + dataDir
    
    filesInSrcDir = os.listdir(currentDir)
    filesToConvert = []
    
    for optionsFile in filesInSrcDir:
        if optionsFile[-5:] == ".yaml" and optionsFile.find("options") == 0:
            optionsSrcPath = os.path.join(currentDir, optionsFile)
            optionsDestPath = os.path.join(dataDir, optionsFile)
            if os.path.exists(optionsDestPath):
                os.remove(optionsDestPath)
            shutil.copy(optionsSrcPath, optionsDestPath)
            print "copied from " + optionsSrcPath + " to " + optionsDestPath
            filesToConvert.append(optionsFile)
    
    # TODO: use copied files
    filesToConvert = ["optionsGlobal.yaml", "optionsTablet.yaml", "optionsMiniTablet.yaml", "optionsPhone.yaml"]
    
    executable = executables[platform.system()]
    print "ex: " + executable
    
    os.chdir(pathToOptionsConverter)
    for optionsFile in filesToConvert:
        optionsRelativePath = "../../Data/" + optionsFile
        params = [executable, optionsRelativePath]
        os.spawnv(os.P_WAIT, executable, params)

    # come back
    os.chdir(currentDir)
        
if __name__ == '__main__':
    main_convert_graphics()