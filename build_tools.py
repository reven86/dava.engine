#!/usr/bin/env python
#

import sys
import os
import platform
import subprocess
import shutil
import argparse


def create_folder(toolsetBuildPath):
    if os.path.exists(toolsetBuildPath):
        shutil.rmtree(toolsetBuildPath)
    os.makedirs(toolsetBuildPath)


def get_cmake_generator():
    platformName = platform.system()
    if platformName == "Darwin":
        return "Xcode"
    elif platformName == "win32":
        return "Visual Studio 12 Win64"


def get_cmake_executable_path(frameworkPath):
    platformName = platform.system()
    if platformName == "Darwin":
        return frameworkPath + "/Bin/CMake.app/Contents/bin/cmake"
    elif platformName == "win32":
        return frameworkPath + "Bin/cmake/bin/cmake.exe"


def get_re_name():
    platformName = platform.system()
    if platformName == "Darwin":
        return "ResourceEditor.app/Contents/MacOS/ResourceEditor"
    elif platformName == "win32":
        return "ResourceEditor.exe"


def get_qe_name():
    platformName = platform.system()
    if platformName == "Darwin":
        return "QuickEd.app/Contents/MacOS/QuickEd"
    elif platformName == "win32":
        return "ResourceEditor.exe"


def create_toolset(cmakePath, toolsetPath, toolsetBuildPath, cmakeGenerator, toolsetBinaryPath):
    commandLine = [cmakePath, "-G", cmakeGenerator, toolsetPath, "-DUNITY_BUILD=true", 
                       "-B" + toolsetBuildPath, "-DDEPLOY=true", "-DONLY_CONTENT_TOOLS=true",
                       "-DDEPLOY_DIR=" + toolsetBinaryPath]
    print "create_toolset: ", commandLine
    subprocess.call(commandLine)


def build_toolset(cmakePath, toolsetBuildPath):
    configuration = ""
    platformName = platform.system()
    if platformName == "Darwin":
        configuration = "Release"
    elif platformName == "win32":
        configuration = "RelWithDebinfo"

    commandLine = ["cmake", "--build", toolsetBuildPath, "--config", configuration]

    print "build_toolset: ", commandLine
    subprocess.call(commandLine)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--test', dest='selfTest', action='store_true')
    parser.add_argument('--no-test', dest='selfTest', action='store_false')
    parser.set_defaults(selfTest=False)
    args = parser.parse_args()

    executablePath = os.path.dirname(sys.argv[0])        
    frameworkPath = os.path.abspath(executablePath)
    toolsetPath = frameworkPath + "/Programs/Toolset"
    toolsetBuildPath = toolsetPath + "/_build"
    toolsetBinaryPath = frameworkPath + "/Bin/Toolset"
    cmakePath = get_cmake_executable_path(frameworkPath)

    create_folder(toolsetBuildPath)
    create_folder(toolsetBinaryPath)

    cmakeGenerator = get_cmake_generator()
    create_toolset(cmakePath, toolsetPath, toolsetBuildPath, cmakeGenerator, toolsetBinaryPath)
    build_toolset(cmakePath, toolsetBuildPath)

    if args.selfTest:
        print "Run Tests:"
        commandLineRE = [toolsetBinaryPath + get_re_name(), "--selfTest"]
        subprocess.call(commandLineRE)

        commandLineQE = [toolsetBinaryPath + get_qe_name(), "--selfTest"]
        subprocess.call(commandLineQE)





