#!/usr/bin/env python
#
#  copy.blitz.py
#  DAVA SDK
#
#  Created by Yury Danilov on 6/14/12.
#  Copyright (c) 2012 DAVA Consulting, LLC. All rights reserved.
#



import os;
import sys;
import os.path;
import string;
import sys;
import shutil

def loadSimpleYamlAsMap(fileName):
    f = open(fileName)
    fileLines = f.readlines()
    f.close()
    data = {}
    for fileLine in fileLines:
        strippedLine = string.strip(fileLine)
        if 0 != string.find(strippedLine, "#"):
            lineWords = string.split(strippedLine, ": ")
            if len(lineWords)==2:
                data[lineWords[0]] = string.strip(lineWords[1], '"')
    return data

arguments = sys.argv[1:]

if 0 == len(arguments) or (len(arguments) < 2 or 3 < len(arguments)):
    print "usage: ./copy.blitz.py RelativePathToMapsSrc RelativePathToMapsDest [RelativePathToMapsList]"
    exit(1)
    
currentDir = os.getcwd()

relativePathToMapsSrc = arguments[0]
relativePathToMapsDest = arguments[1]

relativePathToMapsList = '../Data/maps.yaml'

if 3 == len(arguments):
    relativePathToMapsList = arguments[2]
    print "relativePathToMapsList=" + relativePathToMapsList

# read and process maps from maps.yaml
dataMap = loadSimpleYamlAsMap(relativePathToMapsList)
    
for (serverMap, localMap) in dataMap.iteritems():
    if serverMap != "default":
        (mapFolder, mapFileName) = os.path.split(localMap)
        
        blitzDirSrcPath = os.path.realpath(relativePathToMapsSrc + "/" + mapFolder + '/blitz')
        blitzDirDestPath = os.path.realpath(relativePathToMapsDest + "/" + mapFolder + '/blitz')
        
        blitzFiles = os.listdir(blitzDirSrcPath)
        for blitzFile in blitzFiles:
            print "copy " + blitzFile
            blitzSrcFilePath = os.path.realpath(blitzDirSrcPath + "/" + blitzFile)
            blitzDestFilePath = os.path.realpath(blitzDirDestPath + "/" + blitzFile)
            
            print "blitzSrcFilePath=" + blitzSrcFilePath + " blitzDestFilePath=" + blitzDestFilePath
            # copy mapPath/blitz
            if os.path.exists(blitzSrcFilePath):
                if not os.path.exists(blitzDirDestPath):
                    os.makedirs(blitzDirDestPath)
                if os.path.exists(blitzDestFilePath):
                    print "delete " + blitzDestFilePath
                    os.remove(blitzDestFilePath)
                print "copy " + blitzSrcFilePath + " to " + blitzDestFilePath
                shutil.copy(blitzSrcFilePath, blitzDestFilePath)
# come back
os.chdir(currentDir)
