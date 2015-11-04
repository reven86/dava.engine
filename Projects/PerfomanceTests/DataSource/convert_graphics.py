#!/usr/bin/python
#
#  convert_graphics.py
#  DAVA SDK
#
#  Created by Vitaliy Borodovsky on 10/27/08.
#  Copyright (c) 2008 DAVA Consulting, LLC. All rights reserved.
#

import os
import sys
import os.path
import pickle
import zlib
import string
import sys
import platform
import subprocess
import argparse
import texture_dimension_validator as tdv
import time
import shutil

g_allowed_gpu = ['PowerVR_iOS', 'PowerVR_Android', 'tegra', 'mali', 'adreno', 'origin']
g_default_gpu = 'PowerVR_iOS'
g_default_dirs = ['Gfx']
g_default_resdir = None

g_default_framework_path = { "Darwin": "./../../../", "Windows": "./../../../", "Microsoft": "./../../../" }
g_default_context = {'validate' : tdv.g_default_mode,
                     'gpu' : g_default_gpu,
                     'dirs' : g_default_dirs,
                     'framework' : g_default_framework_path[platform.system()],
                     'resources' : g_default_resdir
                    }

def get_input_context():
    parser = argparse.ArgumentParser(description='Convert graphics.')
    parser.add_argument('--validate', nargs='?', choices = tdv.g_allowed_mode,
        default = g_default_context['validate'])
    parser.add_argument('--gpu', nargs='?', choices = g_allowed_gpu,
        default = g_default_context['gpu'])
    parser.add_argument('--dirs', nargs='*',
        default = g_default_context['dirs'])
    parser.add_argument('--framework', nargs='?',
        default = g_default_context['framework'])
    parser.add_argument('--resources', nargs='?', 
        default = g_default_context['resources'])
    return vars(parser.parse_args())


#-------------------------------
#---Specific for ConvertedGfx---

#merges first dir into the second
def merge_dirs(srcDir, dstDir):
    if not os.path.isdir(srcDir):
        return
        
    for root, dirs, files in os.walk(srcDir):
        dstPath = root.replace(srcDir, dstDir)
        if not os.path.isdir(dstPath):
            shutil.move(root, dstPath)
        else:
            for file in files:
                shutil.move(os.path.join(root, file), os.path.join(dstPath, file))
    
def move_md5(fromRoot, toRoot):
    for fromPath, dirs, files in os.walk(fromRoot):
        if "dir.md5" in files:
            toPath = fromPath.replace(fromRoot, toRoot)
            fromFilePath = os.path.join(fromPath, "dir.md5")
            toFilePath = os.path.join(toPath, "dir.md5")
            
            if not os.path.isdir(toPath):
                os.makedirs(toPath)
            elif os.path.isfile(toFilePath):
                os.remove(toFilePath)
            shutil.move(fromFilePath, toFilePath)
    
#assembles /Data/<gfx> and /$process/<gfx> from /ConvertedGfx/<gpu>/<gfx>
def load_converted_gfx(dataSourcePath, gfxDir, resDir, gpu):
    if gpu is None or resDir is None:
        return

    processPath = os.path.join(dataSourcePath, "$process")
    convertedPath = os.path.join(dataSourcePath, "..", "..", resDir, "ConvertedGfx")
    dataPath = os.path.join(dataSourcePath, "..", "Data")
    
    convertedGpuPath = os.path.join(convertedPath, gpu)
    
    processGfxPath = os.path.join(processPath, gfxDir)
    convertedGfxPath = os.path.join(convertedGpuPath, gfxDir)
    dataGfxPath = os.path.join(dataPath, gfxDir)

    #remove old /Data/<gfx>
    if os.path.isdir(dataGfxPath):
        shutil.rmtree(dataGfxPath)

    #assemblde /Data/<gfx>
    if os.path.isdir(convertedGfxPath):
        shutil.move(convertedGfxPath, dataGfxPath)
    
    #assemble /$process

    #remove /$process/<gfx> dir, remain /$process dir
    if not os.path.isdir(processPath):
        os.makedirs(processPath)
    elif os.path.isdir(processGfxPath):
        shutil.rmtree(processGfxPath)

    #move <gfx>.md5 to /$process
    gfxMD5Name = gfxDir.lower() + ".md5"
    convertedGfxMD5Path = os.path.join(convertedGpuPath, gfxMD5Name)
    processGfxMD5Path = os.path.join(processPath, gfxMD5Name)
    if os.path.isfile(convertedGfxMD5Path):
        if os.path.isfile(processGfxMD5Path):
            os.remove(processGfxMD5Path)
        shutil.move(convertedGfxMD5Path, processGfxMD5Path)

    #move md5 files from /Data/<gfx> to /$process/<gfx>
    move_md5(dataGfxPath, processGfxPath)

#retreives converted data from /Data/<gfx> and /$process/<gfx> to /ConvertedGfx/<gpu>/<gfx>
def save_converted_gfx(dataSourcePath, gfxDir, resDir, gpu):
    if gpu is None or resDir is None:
        return

    processPath = os.path.join(dataSourcePath, "$process")
    convertedPath = os.path.join(dataSourcePath, "..", "..", resDir, "ConvertedGfx")
    dataPath = os.path.join(dataSourcePath, "..", "Data")
    
    convertedGpuPath = os.path.join(convertedPath, gpu)
    
    processGfxPath = os.path.join(processPath, gfxDir)
    convertedGfxPath = os.path.join(convertedGpuPath, gfxDir)
    dataGfxPath = os.path.join(dataPath, gfxDir)

    #copy everything from Data/<gfx> to ConvertedGfx/<gpu>/<gfx>
    if os.path.isdir(convertedGfxPath):
        shutil.rmtree(convertedGfxPath)
    shutil.copytree(dataGfxPath, convertedGfxPath)
    
    #move md5 files from process to ConvertedGfx/<gpu>/<gfx>
    move_md5(processGfxPath, convertedGfxPath)
    
    #move <gfx>.md5 to ConvertedGfx/
    gfxMD5Name = gfxDir.lower() + ".md5"
    convertedGfxMD5Path = os.path.join(convertedGpuPath, gfxMD5Name)
    processGfxMD5Path = os.path.join(processPath, gfxMD5Name)
    if os.path.isfile(processGfxMD5Path):
        if os.path.isfile(convertedGfxMD5Path):
            os.remove(convertedGfxMD5Path)
        shutil.move(processGfxMD5Path, convertedGfxMD5Path)

#---Specific for ConvertedGfx---
#-------------------------------
    
def do(context=g_default_context): # it's function for external usage
    startTime = time.time()

    print context

    currentDir = os.getcwd();
    dataDir =  os.path.realpath(currentDir + "/../Data/")
    print "*** DAVA SDK Launching command line packer - data directory:" + dataDir

    subprocess.call([sys.executable, 'touch_Data.py'])

    pvrTexToolPathname = os.path.normpath(os.path.join(context['framework'], "Tools/Bin"))
    pvrTexToolPathNameRel = os.path.relpath(pvrTexToolPathname)
    os.chdir(pvrTexToolPathname)

    for dir in context['dirs']:
        load_converted_gfx(currentDir, dir, context['resources'], context['gpu'])
        absPath = os.path.realpath(os.path.join(currentDir, dir))
        params = ["./ResourcePacker", os.path.normpath(absPath), pvrTexToolPathNameRel, '-gpu', context['gpu']]
        print "[%s]" % ", ".join(map(str, params))
        os.spawnv(os.P_WAIT, "./ResourcePacker", params)
        save_converted_gfx(currentDir, dir, context['resources'], context['gpu'])

    # come back
    os.chdir(currentDir)

    #run texture dimension validator
    context = {'validate' : context['validate'],
        'data' : './../Data',
        'data_source' : './', 'dirs' : context['dirs']}
    tdv.do(context)

    endTime = time.time()
    print "convert_graphics.py processing time: %f" % (endTime - startTime)


if __name__ == '__main__':
    context = get_input_context()
    do(context)