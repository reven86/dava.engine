#!/usr/bin/python
import sys
import subprocess
import os
import re
import platform
import argparse
from subprocess import call

g_framework_path = ""
g_toolchains_full_path = ""
g_toolchains_relative_framework_path = "Sources/CMake/Toolchains/"
g_ios_toolchain = "ios.toolchain.cmake"
g_android_toolchain = "android.toolchain.cmake"

g_cmake_file_path = ""
g_generation_dir = ""
g_supported_platforms = ["macos", "ios", "android", "windows"]
g_supported_additional_parameters = ["console", "uap", "x64"]
g_is_console = False
g_is_uap = False
g_is_x64 = False
g_is_unity_build = False

def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

def search_program(program):
    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        folders = os.environ["PATH"].split(os.pathsep)
        folders.append("/Applications/CMake.app/Contents/bin")
        for path in folders:
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if platform.system() == 'Windows':
                exe_file = exe_file + '.exe'
            if is_exe(exe_file):
                return exe_file

    return False

def parse_additional_params(additional):
    global g_is_console
    global g_is_uap
    global g_is_x64
    global g_is_unity_build

    if not additional:
        return True;

    for param in additional:
        param = param.lower()

        if "console" == param:
            g_is_console = True
        elif "uap" == param:
            g_is_uap = True
        elif "x64" == param:
            g_is_x64 = True
        elif "ub" == param:
            g_is_unity_build = True                
        else:
            print "Unsupported additional parameter " + "'" + param + "'" + " Use combination of " + str(g_supported_additional_parameters)
            return False

    return True


def setup_framework_env():
    global g_framework_path
    global g_toolchains_full_path
    global g_toolchains_relative_framework_path
    global g_cmake_file_path

    # take current path and make it easier understandable
    path = os.path.realpath(sys.argv[0])
    path = path.replace('\\', '/')
    path = os.path.normpath(path)
    path = path if path.endswith('/') else path + '/'

    # take path like "some/folder/dava.framework.something/inside/"
    # and split it to "some/folder/" + "dava.framework.something/" + "inside/"
    regex = r"(dava\.framework[^/]*/)"
    tail = re.split(regex, path)

    # determine full path to dava.framework folder
    g_framework_path = ""
    if len(tail) >= 3:
        g_framework_path = tail[0] + tail[1]

    if "" == g_framework_path:
        return False

    g_toolchains_full_path = os.path.realpath( g_framework_path + g_toolchains_relative_framework_path )
    g_toolchains_full_path = g_toolchains_full_path + '/'

    return True


def get_project_type(dst_platform, is_console):
    dst_platform = dst_platform.lower()
    project_string = ""

    if "macos" == dst_platform or "ios" == dst_platform:
        project_string += "Xcode"

    if "windows" == dst_platform:
        project_string += "Visual Studio 12"

    if "android" == dst_platform:
        current_platform = platform.system()

        if not g_is_console:
            project_string += "Eclipse CDT4 - "

        if "MinGW" == current_platform:
            project_string += "Mingw Makefiles"
        elif "Windows" == current_platform:
            project_string += "NMake Makefiles"
        else:
            project_string += "Unix Makefiles"

    return project_string


def get_toolchain(input_platform, input_project_type):
    global g_ios_toolchain
    global g_android_toolchain
    global g_is_x64

    toolchain_base = "-DCMAKE_TOOLCHAIN_FILE=" + g_toolchains_full_path;
    toolchain_string = ""
    output_project = input_project_type

    if "ios" == input_platform:
        toolchain_string = toolchain_base + g_ios_toolchain

    if "android" == input_platform:
        toolchain_string = toolchain_base + g_android_toolchain

    if "windows" == input_platform and g_is_x64:
        output_project += " Win64";

    return toolchain_string, output_project

def main():
    global g_supported_additional_parameters
    global g_supported_platforms

    parser = argparse.ArgumentParser(description='Dava.Framework projects genarator')
    parser.add_argument('platform_name', help='One of ' + str(g_supported_platforms))
    parser.add_argument('additional_params', nargs='*', help= 'One of ' + str(g_supported_additional_parameters))
    parser.add_argument('cmake_path', help='relative path to cmake list')
    parser.add_argument('--generation_dir', default="", help="path to generation cmake list" )
    parser.add_argument('--add_definitions', '-defs', default="", help="add definitions" )
    parser.add_argument('--x64', default=False )
    parser.add_argument('-D', action='append', default=[], help="add definitions" )

    options = parser.parse_args()

    if not setup_framework_env():
        print "Couldn't configure environment. Make sure that you runs this script from dava.framework subfolder."
        exit()

    destination_platform = ""

    g_is_x64          = options.x64
    g_cmake_file_path = os.path.realpath(options.cmake_path)
    g_generation_dir  = options.generation_dir
    g_add_definitions = options.add_definitions.replace(',',' ')

    if options.platform_name not in g_supported_platforms:
        print "Wrong destination OS name " + "'" + options.platform_name + "'"
        parser.print_help()
        exit();
    else:
        destination_platform = options.platform_name.lower()

    if False == parse_additional_params(options.additional_params):
        parser.print_help()
        exit()

    project_type = get_project_type(destination_platform, g_is_console)
    if project_type == "":
        print "Unknown project type. Seems get_project_type() works wrong."
        exit()


    toolchain, project_type = get_toolchain(destination_platform, project_type)

    g_cmake_file_path = os.path.realpath(options.cmake_path)

    g_generation_dir  = options.generation_dir
    g_add_definitions = options.add_definitions.replace(',',' ')

    if len(g_generation_dir) :
        if not os.path.exists(g_generation_dir):
            os.makedirs(g_generation_dir)
        os.chdir( g_generation_dir )

    cmake_program = search_program("cmake")

    if False == cmake_program:
        print "cmake command not found."
        exit()

    call_string = [cmake_program, '-G', project_type, toolchain, g_cmake_file_path]

    if len(options.add_definitions):
        call_string += options.add_definitions.split(',') 

    if len(options.D):
        call_string += map(lambda val: '=' in val and '-D'+val or '-D'+val+'=true', options.D )
    
    if g_is_unity_build:
        call_string.append("-DUNITY_BUILD=true")
    print call_string

    call(call_string)
    
    if "android" == destination_platform:
        call(call_string)    

if __name__ == '__main__':
    main()

