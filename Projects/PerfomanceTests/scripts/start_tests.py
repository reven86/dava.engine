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
import argparse
import re
import sys
import os.path
import signal


def get_postfix(platform):
    if platform == 'win32':
        return '.exe'
    elif platform == 'darwin':
        return '.app'
    else:
        return ''

PRJ_NAME_BASE = "PerformanceTests"
PRJ_POSTFIX = get_postfix(sys.platform)

parser = argparse.ArgumentParser(description='Start tests')
parser.add_argument('--branch', nargs='?', default = 'development')
parser.add_argument('--platform', nargs='?', default = 'android')

args = vars(parser.parse_args())

start_on_android = False
start_on_ios = False

if args['platform'] == "android":
    start_on_android = True
elif args['platform']  == "ios":
    start_on_ios = True

sub_process = None

def start_unittests_on_android_device():
    global sub_process
    # if screen turned off
    device_state = subprocess.check_output(['adb', 'shell', 'dumpsys', 'power'])
    if device_state.find("mScreenOn=false") != -1:
        # turn screen on
        subprocess.check_call(['adb', 'shell', 'input', 'keyevent', '26'])
    # unlock device screen
    subprocess.check_call(['adb', 'shell', 'input', 'keyevent', '82'])
    # clear log before start tests
    subprocess.check_call(["adb", "logcat", "-c"])
    # start adb logcat and gather output DO NOT filter by TeamcityOutput tag
    # because we need interrupt gather log when unittests process finished
    sub_process = subprocess.Popen(
        ["adb", "logcat", "-s", "TeamcityOutput"],
        stdout=subprocess.PIPE)
    # start unittests on device
    subprocess.Popen(
        ["adb", "shell", "am", "start", "-n", "com.dava.performancetests/com.dava.performancetests." + PRJ_NAME_BASE])
    return sub_process


if start_on_ios:
    # ../build/ios-deploy -d --noninteractive -b ../build/UnitTests.app
    sub_process = subprocess.Popen(["./ios-deploy", "-d", "--noninteractive", "-b", "../build/" +
                                    PRJ_NAME_BASE + PRJ_POSTFIX],
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print("copy " + PRJ_NAME_BASE + PRJ_POSTFIX + " on device and run")
elif start_on_android:
    sub_process = start_unittests_on_android_device()
elif sys.platform == 'win32':
    if os.path.isfile("..\\Release\\app\\" + PRJ_NAME_BASE + PRJ_POSTFIX):  # run on build server (TeamCity)
        sub_process = subprocess.Popen(["..\\Release\\app\\" + PRJ_NAME_BASE + PRJ_POSTFIX], cwd="./..",
                                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        sub_process = subprocess.Popen(["..\\Release\\" + PRJ_NAME_BASE + PRJ_POSTFIX], cwd="./..",
                                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
elif sys.platform == "darwin":
    if os.path.exists("./" + PRJ_NAME_BASE + PRJ_POSTFIX):
        # if run on teamcity current dir is: Projects/UnitTests/DerivedData/TemplateProjectMacOS/Build/Products/Release
        app_path = "./" + PRJ_NAME_BASE + PRJ_POSTFIX + "/Contents/MacOS/" + PRJ_NAME_BASE
    else:
        # run on local machine from dir: UnitTests/Report
        # Warning! To make DerivedData relative to project go to
        # Xcode->Preferences->Location->DerivedData select relative
        app_path = "../DerivedData/TemplateProjectMacOS/Build/Products/Release/PerformanceTests.app/Contents/MacOS/" \
                   + PRJ_NAME_BASE
    sub_process = subprocess.Popen([app_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

app_exit_code = None

branch = args['branch']
branch = branch.replace("/", "_")

if not os.path.exists("../artifacts"):
    os.makedirs("../artifacts")

continue_process_stdout = True

while continue_process_stdout:
    try:
        line = sub_process.stdout.readline()
        if line != '':

            teamcity_line_index = line.find("Device")
            if teamcity_line_index != -1:
                teamcity_line = line[teamcity_line_index:]
                device = teamcity_line.split("{")[1].split("}")[0]
                frame_delta_file = open("../artifacts/frame_delta" + "_branch_" + branch + "_device_" + device + ".txt", "w")

            # write Frame_delta build statistic to file
            teamcity_line_index = line.find("##teamcity[buildStatisticValue key='Frame_delta'")
            if teamcity_line_index != -1:
                teamcity_line = line[teamcity_line_index:]
                frame_delta_file.write(teamcity_line)
            else:
                # append build statistic keys for compare on teamcity
                teamcity_line_index = line.find("buildStatisticValue key")
                if teamcity_line_index != -1:
                    teamcity_line = line[teamcity_line_index:]
                    key = teamcity_line.split("key")[1].split("'")[1]
                    value = teamcity_line.split("value")[1].split("'")[1]

                    key = key + "_branch_" + branch + "_device_" + device

                    sys.stdout.write("##teamcity[buildStatisticValue key='" + key + "' value='" + value + "']")
                    sys.stdout.flush()          
                else:
                    teamcity_line_index = line.find("##teamcity")
                    if teamcity_line_index != -1:
                        teamcity_line = line[teamcity_line_index:]
                        sys.stdout.write(teamcity_line)
                        sys.stdout.flush()

            if line.find("Finish all tests.") != -1:    # this text marker helps to detect good \
                                    
                app_exit_code = 0
                frame_delta_file.close()

                if start_on_android:
                    # we want to exit from logcat process because sub_process.stdout.readline() will block
                    # current thread
                    if sys.platform == "win32":
                        sub_process.send_signal(signal.CTRL_C_EVENT)
                    else:
                        sub_process.send_signal(signal.SIGINT)
                    continue_process_stdout = False
        else:
            continue_process_stdout = False
    except IOError as err:
        sys.stdout.write(err.message)
        sys.stdout.flush()

if app_exit_code is None:
    app_exit_code = sub_process.poll()

sys.exit(app_exit_code)
