import os
import sys

if len(sys.argv) > 1:
	branch = sys.argv[1]
else:
	branch = ' '
	

os.chdir('../')
os.system("git config --global log.date local")
os.system("git log -1 --format=\"#define RESOURCE_EDITOR_VERSION %x22%cd | " + branch + "%x22\" > version.h")

os.chdir('../../')
os.system("git log -1 --format=\"%ci\" > gitTime.txt")
