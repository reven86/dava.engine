import os
import sys
from subprocess import Popen, PIPE
from shutil import copytree, rmtree, ignore_patterns

if len(sys.argv) < 2:
    print "Usage : postDeploy.py <deploy_root_folder> [beast_lib_folder]"
    exit()

deployRoot = sys.argv[1]

proc = Popen("git log --since=3.days --branches=\"development\" --pretty=format:\"%s (%an, %ar)\"", shell=True, stdout=PIPE)
changesFile = open(os.path.join(deployRoot, "changes.txt"), 'w+')
for line in proc.stdout:
    changesFile.write(line)

if len(sys.argv) > 2:
    beastFolderPath = sys.argv[2].rstrip("\\/")
    beastDstPath = os.path.join(deployRoot, os.path.basename(beastFolderPath))
    if os.path.exists(beastDstPath):
        rmtree(beastDstPath)
    IGNORE_PATTERNS = ('.svn')
    copytree(beastFolderPath, beastDstPath, symlinks=False, ignore=ignore_patterns(IGNORE_PATTERNS))
