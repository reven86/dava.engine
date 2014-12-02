#!/usr/bin/env python

# start application on device or PC(Mac) 
# then start server inside this script to listen unit test log output
# example:
# win32: 
# > cd to dava.framework/Projects/UnitTests/Reports
# > python start_unit_tests.py
# mac os x: 
# make DerivedData relative to project go to Xcode->Preferences->Location->DerivedData select relative
# build project
# > cd to dava.framework/Projects/UnitTests/Reports
# > python start_unit_tests.py
# android:
# deploy application to device first
# > python start_unit_tests.py android
# iOS:
# deploy application on device first
# "Enable UI Automation" must be turned on in the "Developer" section of the "Settings" app on device
# > python start_unit_tests.py ios

import subprocess
import sys
import os.path


def get_postfix(platform):
    if platform == 'win32':
        return '.exe'
    elif platform == 'darwin':
        return '.app'
    else:
        return ''


PRJ_NAME_BASE = "UnitTests"
PRJ_POSTFIX = get_postfix(sys.platform)

start_on_android = False
start_on_ios = False

if len(sys.argv) > 1:
    if sys.argv[1] == "android":
        start_on_android = True
    elif sys.argv[1] == "ios":
        start_on_ios = True

sub_process = None

if start_on_ios:
    # ../build/ios-deploy -d --noninteractive -b ../build/UnitTests.app
    sub_process = subprocess.Popen(["../build/ios-deploy", "-d", "--noninteractive", "-b", "../build/UnitTests.app"],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
elif start_on_android:
    sub_process = subprocess.Popen(
        ["adb", "shell", "am", "start", "-n", "com.dava.unittests/com.dava.unittests." + PRJ_NAME_BASE])
elif sys.platform == 'win32':
    if os.path.isfile("..\\Release\\app\\" + PRJ_NAME_BASE + PRJ_POSTFIX):  # run on build server (TeamCity)
        sub_process = subprocess.Popen(["..\\Release\\app\\" + PRJ_NAME_BASE + PRJ_POSTFIX], cwd="./..")
    else:
        sub_process = subprocess.Popen(["..\\Release\\" + PRJ_NAME_BASE + PRJ_POSTFIX], cwd="./..")
elif sys.platform == "darwin":
    if os.path.exists("./" + PRJ_NAME_BASE + PRJ_POSTFIX):
        # if run on teamcity current dir is: Projects/UnitTests/DerivedData/TemplateProjectMacOS/Build/Products/Release
        app_path = "./" + PRJ_NAME_BASE + PRJ_POSTFIX
    else:
        # run on local machine from dir: UnitTests/Report
        # Warning! To make DerivedData relative to project go to
        # Xcode->Preferences->Location->DerivedData select relative
        app_path = "../DerivedData/TemplateProjectMacOS/Build/Products/Release/UnitTests.app/Contents/MacOS/" \
                   + PRJ_NAME_BASE
    sub_process = subprocess.Popen([app_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

app_exit_code = None

continue_process_stdout = True

while continue_process_stdout:
    try:
        line = sub_process.stdout.readline()
        if line != '':
            teamcity_line_index = line.find("##teamcity")
            if teamcity_line_index != -1:
                teamcity_line = line[teamcity_line_index:]
                sys.stdout.write(teamcity_line)
                sys.stdout.flush()
            if line.find("Finish all tests.") != -1:    # this text marker helps to detect good \
                                                        #  finish tests on ios device (run with lldb)
                app_exit_code = 0
        else:
            continue_process_stdout = False
    except IOError:
        sys.stdout.write(IOError.message)
        sys.stdout.flush()

if app_exit_code is None:
    app_exit_code = sub_process.poll()

sys.exit(app_exit_code)
