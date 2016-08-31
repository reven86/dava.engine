#!/usr/bin/env python

import os
import sys
import platform
import shutil
import stat
from subprocess import Popen, PIPE

def runProcess(args):
    process = Popen(args,  shell=True, stdout=PIPE)
    for line in iter(process.stdout.readline,''):
        print line

def makeDirAndGo(dirName):
    if not os.path.isdir(dirName):
        os.mkdir(dirName)
    os.chdir(dirName)

def copyLib(src, dst):
    if not os.path.isdir(dst):
        os.makedirs(dst)
    shutil.copy2(src, dst)

def copyFolder(src, dst):
    if os.path.isdir(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)

def on_rm_error(func, path, exc_info):
    # path contains the path of the file that couldn't be removed
    # let's just assume that it's read-only and unlink it.
    os.chmod(path, stat.S_IWRITE)
    os.unlink(path)

libsFolder = os.path.join(os.getcwd(), "..")

runProcess("git clone https://github.com/google/googletest.git")

os.chdir("googletest")
copyFolder("googlemock/include/gmock", os.path.join(libsFolder, "Include/gmock"))
copyFolder("googletest/include/gtest", os.path.join(libsFolder, "Include/gtest"))

makeDirAndGo("_build")

if (platform.system() == "Windows"):
    makeDirAndGo("x86")
    runProcess("cmake -G\"Visual Studio 12\" ../..")
    runProcess("cmake --build . --config Debug")
    copyLib("googlemock/Debug/gmock.lib", os.path.join(libsFolder, "Win32/x86/Debug/"))
    runProcess("cmake --build . --config Release")
    copyLib("googlemock/Release/gmock.lib", os.path.join(libsFolder, "Win32/x86/Release/"))

    os.chdir("..")
    makeDirAndGo("x64")
    runProcess("cmake -G\"Visual Studio 12 Win64\" ../..")
    runProcess("cmake --build . --config Debug")
    copyLib("googlemock/Debug/gmock.lib", os.path.join(libsFolder, "Win32/x64/Debug/"))
    runProcess("cmake --build . --config Release")
    copyLib("googlemock/Release/gmock.lib", os.path.join(libsFolder, "Win32/x64/Release/"))
    os.chdir("../../..")
elif (platform.system() == "Darwin"):
    makeDirAndGo("Debug")
    runProcess("cmake  ../..")
    runProcess("cmake --build . --config Debug")
    copyLib("googlemock/libgmock.a", os.path.join(libsFolder, "Mac/Debug/"))

    os.chdir("..")
    makeDirAndGo("Release")
    runProcess("cmake  ../..")
    runProcess("cmake --build . --config Release")
    copyLib("googlemock/libgmock.a", os.path.join(libsFolder, "Mac/Release/"))
    os.chdir("../../..")

shutil.rmtree("googletest", onerror=on_rm_error)

