import sys
import os

scriptName = sys.argv[0]

dir = os.path.dirname(scriptName)
selftestsDir = os.path.join(dir, "Selftests").replace("\\", "/")
includeBaseDir = selftestsDir.split("Sources/")[1]

selftests = [f for f in os.listdir(selftestsDir) if os.path.isfile(os.path.join(selftestsDir, f)) and f.endswith(".h")]

indexFilePath = os.path.join(selftestsDir, "index")
if os.path.exists(indexFilePath) and os.path.isfile(indexFilePath):
    indexFile = open(indexFilePath, "r")
    indexContent = []
    for line in indexFile:
        indexContent.append(line.rstrip("\n"))
    indexFile.close()

    if selftests == indexContent:
        exit(0)

indexFile = open(indexFilePath, "w")
for testFile in selftests:
    indexFile.write(testFile + "\n")
indexFile.close()

deadCodeTrickFile = open(os.path.join(dir, "DeadCodeTrick.cpp"), "w")

deadCodeTrickFile.write("#include \"DeadCodeTrick.h\"\n")
for testFile in selftests:
    deadCodeTrickFile.write("#include \"" + includeBaseDir + "/" + testFile + "\"\n")

deadCodeTrickFile.write("\nnamespace DAVA\n{\nnamespace TArc\n{\n")
deadCodeTrickFile.write("bool AvoidTestsStriping()\n{\n    return true;\n}\n")
deadCodeTrickFile.write("} // namespace TArc\n} // namespace DAVA")
deadCodeTrickFile.close()