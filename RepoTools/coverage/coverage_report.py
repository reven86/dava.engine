#!/usr/bin/env python
import sys
import os
import fnmatch
import argparse
import subprocess
import shutil
import json
import re

CoverageTmpDirName = 'CoverageTmpData';

def enum(**enums):
    return type('Enum', (), enums)

def find_file(pattern, dirs):
    result = []
    for path in dirs:
        for root, dirs, files in os.walk(path):
            for name in files:
                if fnmatch.fnmatch(name, pattern):
                    result.append(os.path.join(root, name))
    return result

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

def run_executable( pathExecut ) :
    print '-->',get_exe( pathExecut )

    pathExecutDir = os.path.dirname ( pathExecut )
    pathExecut    = get_exe( pathExecut )

    os.chdir( pathExecutDir )
    sub_process = subprocess.Popen([pathExecut], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    subProcessContinue = True

    while subProcessContinue:
        try:
            line = sub_process.stdout.readline()
            sys.stdout.write(line)
            sys.stdout.flush()
            if line == '':
                subProcessContinue = False
                continue
        except IOError as err:
            sys.stdout.write(err.message)
            sys.stdout.flush()


def coppy_gcda_gcno_files( pathExecut, pathBuild, coverageTmpPath, buildConfig ):
    executName            = os.path.basename( pathExecut )
    executName, ExecutExt = os.path.splitext( executName )

    if not os.path.isdir( coverageTmpPath ):
        os.makedirs(coverageTmpPath) 

    if buildConfig:
        pathConfigSegment = os.path.join(  '{0}.build'.format(executName), buildConfig )   

    #coppy '.gcda','.gcno' files
    listCoverData = []
    for rootdir, dirs, files in os.walk( pathBuild ):
        if rootdir.find( coverageTmpPath ) == -1 :
            if not buildConfig or rootdir.find( pathConfigSegment ) != -1:
                for file in files:   
                    if file.endswith( ('.gcda','.gcno')  ): 
                        listCoverData += [os.path.join(rootdir, file)]
    
    for file in listCoverData:
        baseName = os.path.basename( file )
        pathOutFile = os.path.join(coverageTmpPath, baseName)

        if os.path.exists(pathOutFile):
            os.remove(pathOutFile)

        shutil.copy(file, coverageTmpPath )


def generate_report_html( pathExecut, coverageTmpPath, pathReportOut ):

    pathExecutDir         = os.path.dirname ( pathExecut )
    executName            = os.path.basename( pathExecut )
    executName, ExecutExt = os.path.splitext( executName )

    if os.path.isdir( pathReportOut ):
        shutil.rmtree( pathReportOut )
    else:
        os.makedirs(pathReportOut)      

    #
    pathCoverageDir  = os.path.dirname(os.path.realpath(__file__))
    pathCallLlvmGcov = os.path.join(pathCoverageDir, 'llvm-gcov.sh')
    pathCovInfo      = os.path.join(coverageTmpPath, 'cov.info')
    pathLcov         = os.path.join(pathCoverageDir, 'lcov')
    pathGenHtml      = os.path.join(pathCoverageDir, 'genhtml')

    os.chdir( pathCoverageDir ) 

    #
    params = [ pathLcov,
                '--directory', coverageTmpPath,  
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

def generate_report_coverage( pathExecut, coverageTmpPath ):

    class FileCover():
        def __init__(self, fileName, coverLine):
            self.file = fileName
            self.coverLine = coverLine

    eState = enum( UNDEF      =0, 
                   FIND_File  =1, 
                   FIND_Lines =2, 
                   FIND_Taken =3 )

    pathExecutDir         = os.path.dirname ( pathExecut )
    executName            = os.path.basename( pathExecut )
    executName, ExecutExt = os.path.splitext( executName )

    pathCoverageDir       = os.path.dirname(os.path.realpath(__file__))
    pathLlvmCov           = os.path.join(pathCoverageDir, 'llvm-cov')

    coverFilePath         = os.path.join( pathExecutDir, '{0}.cover'.format( executName ) )

    if not os.path.isfile(coverFilePath) or not os.access(coverFilePath, os.R_OK):
        print 'ERROR : file {0} is missing or is not readable'.format( coverFilePath )
        return

    coverFile      = open(coverFilePath).read()
    jsonData       = json.loads(coverFile)

    projectFolders = jsonData[ 'ProjectFolders' ].split(' ')
    testsCoverage  = {}

    for test in jsonData[ 'Coverage' ]:
        testedFiles = jsonData[ 'Coverage' ][ test ].split(' ')
        for file in testedFiles:
            for ext in [ 'cpp', 'c']:
                find_list = find_file( '{0}.{1}'.format( file, ext ) , projectFolders )
                if len( find_list ):
                    fileCover = FileCover( find_list[0], None )
                    testsCoverage.setdefault(test, []).append( fileCover )

    os.chdir( coverageTmpPath );
    for test in testsCoverage:
        state = eState.FIND_File

        for fileCover in testsCoverage[ test ]:
            fileName          = os.path.basename( fileCover.file )
            fileName, fileExt = os.path.splitext( fileName )

            fileGcda     = '{0}.gcda'.format(fileName)
            fileGcdaList = [ ]

            if  os.path.isfile( fileGcda ) :
                fileGcdaList = [ fileGcda ]
            else:
                for rootdir, dirs, files in os.walk( coverageTmpPath ):
                    for file in files:   
                        if file.endswith( ('.gcda')  ): 
                            fileGcdaList += [os.path.join(rootdir, file)]

            for fileGcda in fileGcdaList:

                params = [ pathLlvmCov, 'gcov',
                    '-f',  
                    '-b', 
                    fileGcda
                     ]

                subProcess         = subprocess.Popen(params, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                subProcessContinue = True

                while subProcessContinue:
                    try:
                        line = subProcess.stdout.readline()
                        split_line = line.split( )

                        if line == '':
                            subProcessContinue = False
                            continue

                        if len( split_line ) == 0:
                            continue

                        if   ( state == eState.FIND_File 
                               and split_line[0] == 'File' 
                               and split_line[1].replace('\'','') == fileCover.file ):
                            state = eState.FIND_Lines

                        elif ( state == eState.FIND_Lines 
                               and split_line[0] == 'Lines' ):
                            fileCover.coverLine = re.findall("\d+\.\d+", split_line[1] )[0]
                            state = eState.FIND_File
                            subProcessContinue = False
                            subProcess.kill()

                    except IOError as err:
                        sys.stdout.write(err.message)
                        sys.stdout.flush()

    for test in testsCoverage:
        print test
        for fileCover in testsCoverage[ test ]:
            print fileCover.file,' ',fileCover.coverLine
        print ''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument( '--pathBuild', required = True )
    parser.add_argument( '--pathExecut', required = True )
    parser.add_argument( '--pathReportOut', required = True )
    parser.add_argument( '--buildConfig', choices=['Debug', 'Release'] )
    parser.add_argument( '--notRunExecutable', default = 'false', choices=['true', 'false'] )

    options = parser.parse_args()

    if options.notRunExecutable == 'false' :
        run_executable( options.pathExecut )
    
    coverageTmpPath = os.path.join( options.pathBuild, CoverageTmpDirName )

    coppy_gcda_gcno_files( options.pathExecut, options.pathBuild, coverageTmpPath, options.buildConfig )

    generate_report_html(  options.pathExecut, coverageTmpPath, options.pathReportOut )
    generate_report_coverage( options.pathExecut, coverageTmpPath )


if __name__ == '__main__':
    main()


















