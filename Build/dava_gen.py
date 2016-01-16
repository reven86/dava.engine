#!/usr/bin/python
import sys
import subprocess
import os
import re
import platform
import argparse

g_framework_path = ""
g_toolchains_full_path = ""
g_toolchains_relative_framework_path = "Sources/CMake/Toolchains/"
g_ios_toolchain = "ios.toolchain.cmake"
g_android_toolchain = "android.toolchain.cmake"

g_cmake_file_path = ""
g_supported_platforms = ["macos", "ios", "android", "windows"]
g_supported_additional_parameters = ["console", "uap", "x64"]
g_is_console = False
g_is_uap = False
g_is_x64 = False

def parse_additional_params(additional):
    global g_is_console
    global g_is_uap
    global g_is_x64

    if not additional:
        return True;

    for param in additional:
        param = param.lower()

        if "console" == param:
            g_is_console = True
            return True

        if "uap" == param:
            g_is_uap = True
            return True

        if "x64" == param:
            g_is_x64 = True
            return True

        print "Unsupported additional parameter " + "'" + param + "'" + " Use combination of " + str(g_supported_additional_parameters)
        return False

    return False


def setup_framework_env():
    global g_framework_path
    global g_toolchains_full_path
    global g_toolchains_relative_framework_path
    global g_cmake_file_path

    # take current path and make it easier understandable
    path = os.getcwd()
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

    g_toolchains_full_path = g_framework_path + g_toolchains_relative_framework_path

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

        if not is_console:
            project_string += "Eclipse CDT4 - "

        if "MinGW" == current_platform:
            project_string += "Mingw Makefiles"
        if "Windows" == current_platform:
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

    options = parser.parse_args()

    if not setup_framework_env():
        print "Couldn't configure environment. Make sure that you runs this script from dava.framework subfolder."
        exit()

    destination_platform = ""

    if options.platform_name not in g_supported_platforms:
        print "Wrong destination OS name " + "'" + options.platform_name + "'"
        parser.print_help()
        exit();
    else:
        destination_platform = options.platform_name

    project_type = get_project_type(destination_platform, g_is_console)
    if project_type == "":
        print "Unknown project type. Seems get_project_type() works wrong."
        exit()

    if False == parse_additional_params(options.additional_params):
        parser.print_help()
        exit()

    toolchain, project_type = get_toolchain(destination_platform, project_type)

    g_cmake_file_path = options.cmake_path

    call_string = ['cmake', '-G', project_type, toolchain, g_cmake_file_path]

    print call_string

    subprocess.check_output(call_string)

    if "android" == destination_platform:
        subprocess.check_output(call_string)

if __name__ == '__main__':
    main()

