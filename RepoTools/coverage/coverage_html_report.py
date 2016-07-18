#!/usr/bin/env python
import sys
import os
import argparse
import subprocess
import shutil

def get_exe( pathExecut ):

    pathExecut = os.path.realpath( pathExecut )

    if os.name == 'nt':
        return pathExecut
    else:
        # mac os
        dirName  = os.path.dirname ( pathExecut )
        baseName = os.path.basename( pathExecut )
        baseName, extApp = os.path.splitext(baseName)

        if extApp == '.app' :
            return '{0}/{1}.app/Contents/MacOS/{1}'.format( dirName, baseName )
        else:
            return pathExecut

def generate_report_html( pathBuild, pathExecut, pathReportOut ):

    pathExecutDir = os.path.dirname ( pathExecut )

    if not os.path.isdir( pathReportOut ):
        os.makedirs(pathReportOut) 


    print '-->',get_exe( pathExecut )

    subprocess.call( get_exe( pathExecut ) )

    #coppy '.gcda','.gcno' files
    listCoverData = []
    for rootdir, dirs, files in os.walk( pathBuild ):
        if rootdir.find( pathExecutDir ) == -1:
            for file in files:   
                if file.endswith( ('.gcda','.gcno')  ): 
                    listCoverData += [os.path.join(rootdir, file)]
    
    for file in listCoverData:
        baseName = os.path.basename( file )
        pathOutFile = os.path.join(pathExecutDir, baseName)

        if os.path.exists(pathOutFile):
            os.remove(pathOutFile)

        shutil.copy(file, pathExecutDir )

    #
    pathCoverageDir  = os.path.dirname(os.path.realpath(__file__))
    pathCallLlvmGcov = os.path.join(pathCoverageDir, 'llvm-gcov.sh')
    pathCovInfo      = os.path.join(pathExecutDir, 'cov.info')
    pathLcov         = os.path.join(pathCoverageDir, 'lcov')
    pathGenHtml      = os.path.join(pathCoverageDir, 'genhtml')

    os.chdir( pathCoverageDir ); 

    #
    params = [ pathLcov,
                '--directory', pathExecutDir,  
                '--base-directory', pathExecutDir,
                '--gcov-tool', pathCallLlvmGcov,
                '--capture',   
                '-o', pathCovInfo
                 ]
    
    subprocess.call(params)

    #
    params = [ pathGenHtml,
            pathCovInfo, 
            '-o', pathReportOut
             ]

    subprocess.call(params)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument( '--pathBuild', required = True )
    parser.add_argument( '--pathExecut', required = True )
    parser.add_argument( '--pathReportOut', required = True )
    options = parser.parse_args()

    generate_report_html( options.pathBuild, options.pathExecut, options.pathReportOut )
 

if __name__ == '__main__':
    main()


















