#!/bin/python

import socket
import subprocess
import sys

current_host = socket.gethostname()
current_ip = socket.gethostbyname(current_host)

HOST = current_ip
PORT = 50007

print("host ip: " + current_ip + ":" + str(PORT))

# start application on device or else where then start server to listen unit test log output
# TODO move to command line param of script
start_on_android = False
start_on_ios = False

if len(sys.argv) > 1:
    if sys.argv[1] == "android":
        start_on_android = True
    elif sys.argv[1] == "ios":
        start_on_ios = True

if start_on_ios:
    app_name = "com.davaconsulting.UnitTests"
    device_name = "iPadMini1FW" # TODO put your device name here
    test_run_file = "testRun.js" # file with content: var target = UIATarget.localTarget(); target.delay( 7200 );
    subprocess.Popen(["instruments", "-t", 
        "/Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.xrplugin/Contents/Resources/Automation.tracetemplate", 
        "-w", device_name, app_name, "-e",
        "UIASCRIPT", test_run_file, "-host", str(HOST), "-port", str(PORT)])
elif start_on_android:
    subprocess.Popen(["adb", "shell", "am", "start", "-n", "com.dava.unittests/com.dava.unittests.UnitTests",
                      "-e", "-host", str(HOST), "-e", "-port", str(PORT)])
elif sys.platform == 'win32':
    subprocess.Popen(["..\Debug\UnitTestsVS2010.exe", "-host",
                      str(HOST), "-port", str(PORT)], cwd="./..")
elif sys.platform == "darwin":
    app_path = "/Users/user123/Library/Developer/Xcode/DerivedData/TemplateProjectMacOS-bpogfklmgukhlmbnpxhfcjhfiwfq/Build/Products/Debug/UnitTests.app"
    subprocess.Popen(["open", "-a", app_path, "--args", "-host", str(HOST), "-port", str(PORT)])

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
print("start listen")
conn, addr = s.accept()
print("connected by:" + str(addr))

file_content = ""

while 1:
    data = None
    try:
        data = conn.recv(1024)
    except socket.error, msg:
        print(msg)
        break
    if not data:
        break
    file_content += data

conn.close()

# TODO write output to stdout or some predefined file

out_file = open("output.xml", "w")
out_file.write(file_content)

