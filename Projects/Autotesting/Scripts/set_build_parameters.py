# utf-8
#  Wargaming.NET SDK
#
#  Created by Aliaksey Baradulkin on 24/02/2014
#  Copyright(c)1998 Wargaming.NET. All rights reserved.


from sys import argv

from datetime import datetime
from argparse import ArgumentParser


def __saveMapAsSimpleYaml(data):
    with open("id.yaml", 'w') as f:
        f.write("keyedArchive:\n")
        for serverMap, localMap in data.iteritems():
            f.write("    {}:\n".format(serverMap))
            f.write('        string: "{}"\n'.format(localMap))


if __name__ == '__main__':
    argParser = ArgumentParser()
    argParser.add_argument('buildID', type=str)
    argParser.add_argument('branch', type=str)
    argParser.add_argument('branchRev', type=str)
    argParser.add_argument('framework', type=str)
    argParser.add_argument('frameworkRev', type=str)
    argParser.add_argument('-d', '--date', type=str, default=datetime.now().strftime('%Y%m%d'))
    namespace = argParser.parse_args(argv[1:])
    data = {'BuildId': namespace.buildID, 'Branch': namespace.branch, 'BranchRev': namespace.branchRev,
            'Framework': namespace.framework, 'FrameworkRev': namespace.frameworkRev,
            'Date': namespace.date}
    __saveMapAsSimpleYaml(data)