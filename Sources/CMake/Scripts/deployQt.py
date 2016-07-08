import os
import sys
from subprocess import Popen, PIPE
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-p", action="store", dest="platform", help="Platform where deploy processing : WIN, MAC")
parser.add_option("-q", action="store", dest="qtPath", help="Path to Qt root folder")
parser.add_option("-d", action="store", dest="deployRoot", help="Root folder of build. {_build/app} for example")
parser.add_option("-a", action="store", dest="deployArgs", help="Platform specific arguments that put into deplot qt util")

(options, args) = parser.parse_args()

if not options.qtPath or not options.deployRoot or not options.deployArgs or not options.platform:
    parser.print_help()
    exit()

deployUtilName = ""
if options.platform == "WIN":
    os.environ['PATH'] = os.path.join(options.qtPath, "bin")
    deployUtilName = "windeployqt"
elif options.platform == "MAC":
    deployUtilName = options.qtPath.rstrip("\\/") + "/bin/macdeployqt"
else:
    parser.print_help()
    exit()

prevCurrentDir = os.getcwd()
os.chdir(options.deployRoot)

procces = Popen(deployUtilName + " " + options.deployArgs, shell=True, stdout=PIPE)
for line in procces.stdout:
    print line
os.chdir(prevCurrentDir)

