import os;
import sys;
import os.path;
import string;
import sys;
import subprocess;
import platform;
import re;
import io

reload(sys)
sys.setdefaultencoding('utf-8')

def process_files(arg, dirname, names):    
    relPath = os.path.relpath(dirname, deployDir); 
    fullpath = os.path.normpath( dirname + "/");
    for fullname in names:
        pathname = fullpath + "/" + fullname;
        relPath = os.path.relpath(pathname, deployDir); 
        changesFile.write(unicode(relPath + '\n'))
  
if len(sys.argv) < 2:
    print "Usage : createPackageInfo.py <deploy app folder>"
    exit()
    
    
deployDir = sys.argv[1]
print "creating info list";
changesFile = io.open(os.path.join(deployDir, "Launcher.packageInfo"), 'w', newline='')
os.path.walk(deployDir, process_files, None);

changesFile.close()