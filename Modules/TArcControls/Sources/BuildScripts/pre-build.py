import sys
import os

if len(sys.argv) < 2:
    print "Usage : pre-build.py [TArcControlsRootDir]"
    exit(1)

qmlDir = os.path.join(sys.argv[1], "Qml")
outputFilePath = os.path.join(sys.argv[1], "qmldir")

# qmldir file generation
qmlFiles = [f for f in os.listdir(qmlDir) if os.path.isfile(os.path.join(qmlDir, f)) and f.endswith(".qml")]

qmlDirFile = open(outputFilePath, "w")
qmlDirFile.write("module TArcControls\n")
for f in qmlFiles:
    qmlDirFile.write(f.rstrip(".qml") + " 1.0 " + f + "\n")
qmlDirFile.write("plugin TArcControls")
qmlDirFile.close()

#qrc file generation
resourcesDir = os.path.join(sys.argv[1], "Resources")
resourceFiles = []
for root, subdirs, files in os.walk(resourcesDir):
    for f in files:
        if (not f.endswith(".qrc")):
            resourceFiles.append(os.path.join(root, f))

qrcFilePath = os.path.join(resourcesDir, "Resources.qrc")
qrcFile = open(qrcFilePath, "w")
qrcFile.write("<RCC>\n")
qrcFile.write("    <qresource prefix=\"/\">\n")
for rcFile in resourceFiles:
    qrcFile.write("        <file>" + rcFile.lstrip(resourcesDir).replace("\\", "/") + "</file>\n")
qrcFile.write("    </qresource>\n")
qrcFile.write("</RCC>\n")
qrcFile.close()


