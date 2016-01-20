import os
import sys
from subprocess import Popen, PIPE

if len(sys.argv) < 3:
    print "Usage : deployQt.py <chdirArgument> <deployCommand>"
    exit()

prevCurrentDir = os.getcwd()
os.chdir(sys.argv[1])
deployCommand = sys.argv[2]

os.environ['PATH']=sys.argv[1]
procces = Popen(deployCommand, shell=True, stdout=PIPE)
for line in procces.stdout:
    print line
os.chdir(prevCurrentDir)

