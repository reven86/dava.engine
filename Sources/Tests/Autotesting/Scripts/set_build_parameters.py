import sys
import datetime

args = sys.argv[1:]

data = {}
data['buildId'] = args[0]
data['branch']  = args[1]
data['branchRev']   = args[2]
data['framework']   = args[3]
data['frameworkRev']    = args[4]

data['date'] = datetime.datetime.now().strftime('%Y%m%d')

if len(args) != 5 :
    print 'Usage: ./set_build_parameters.py buildId branch branchRev framework frameworkRev'
    exit(1)
    
def saveMapAsSimpleYaml(fileName, data):
    f = open(fileName, 'w')
    f.write("keyedArchive:\n")
    for (serverMap, localMap) in data.iteritems():
        fileLine = "    " + str(serverMap) + ":\n"
        f.write(fileLine)
        fileLine = '        string: "' + str(localMap) + '"\n'
        f.write(fileLine)
    f.close()

file = "id.yaml"

saveMapAsSimpleYaml(file, data)