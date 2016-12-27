import sys
import os


def dumpQmlModule(path, moduleName, isRoot):
    # qmldir file generation
    qmlFiles = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f)) and f.endswith(".qml")]

    qmlDirFile = open(os.path.join(path, "qmldir"), "w")
    qmlDirFile.write("module " + moduleName + "\n")
    for f in qmlFiles:
        qmlDirFile.write(f.rstrip(".qml") + " 1.0 " + f + "\n")
    if isRoot:
        qmlDirFile.write("plugin TArcControls")
    qmlDirFile.close()

if len(sys.argv) < 2:
    print "Usage : pre-build.py [TArcControlsRootDir]"
    exit(1)

qmlDir = os.path.join(sys.argv[1], "TArcControls")
dumpQmlModule(qmlDir, "TArcControls", True)
dumpQmlModule(os.path.join(qmlDir, "Styles"), "TArcControls", False)

#qrc file generation
resourcesDir = os.path.join(sys.argv[1], "Resources")
resourcesDir = resourcesDir.replace("\\", "/")
resourcesDir = resourcesDir.lstrip("/") + "/"
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
    rcFile = rcFile.replace("\\", "/")
    rcFile = rcFile.replace(resourcesDir, "")
    qrcFile.write("        <file>" + rcFile + "</file>\n")
qrcFile.write("    </qresource>\n")
qrcFile.write("</RCC>\n")
qrcFile.close()


