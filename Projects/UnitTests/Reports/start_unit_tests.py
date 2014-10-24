
import socket
import subprocess
import sys

# TODO start application on device or else where then start server to listen unit test log output
# some_command_to_start_app_on_device

if sys.platform == 'win32':
    subprocess.Popen(["..\Debug\UnitTestsVS2010.exe",
                      "127.0.0.1", "50007"], cwd="./..")
elif sys.platform == "darwin":
    # /Users/user123/Library/Developer/Xcode/DerivedData/TemplateProjectMacOS-bpogfklmgukhlmbnpxhfcjhfiwfq/Build/Products/Debug/UnitTests
    app_path = "/Users/user123/Library/Developer/Xcode/DerivedData/TemplateProjectMacOS-bpogfklmgukhlmbnpxhfcjhfiwfq/Build/Products/Debug/UnitTests.app"
    subprocess.Popen(["open", "-a", app_path])  # , "127.0.0.1", "50007"
    # TODO run on mac os x
    pass

HOST = 'localhost'        # Symbolic name meaning the local host
PORT = 50007              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print("connected by:", addr)

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

