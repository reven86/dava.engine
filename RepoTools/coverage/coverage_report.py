#!/usr/bin/env python
import sys
import os
import fnmatch
import argparse
import subprocess
import shutil
import json
import re

CoverageMinimum    = 80.0


class FileCover():
    def __init__(self, fileName, coverLines):
        self.file = fileName
        self.coverLines = coverLines

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


class CoverageReport():

    def __init__(self, arg ):

        self.arg                    = arg
        self.pathBuild              = arg.pathBuild
        self.pathExecut             = arg.pathExecut
        self.pathReportOut          = arg.pathReportOut
        self.buildConfig            = arg.buildConfig
        self.notRunExecutable       = arg.notRunExecutable

        self.coverageTmpPath        = os.path.join( arg.pathBuild, 'CoverageTmpData' )
    
        self.pathExecutDir          = os.path.dirname ( self.pathExecut )
        self.executName             = os.path.basename( self.pathExecut )
        self.executName, ExecutExt  = os.path.splitext( self.executName )
        
        self.pathCoverageDir        = os.path.dirname (os.path.realpath(__file__))

        self.coverFilePath          = os.path.join    ( self.pathExecutDir,   '{0}.cover'.format( self.executName ) )
        self.pathLlvmCov            = os.path.join    ( self.pathCoverageDir, 'llvm-cov')
        self.pathCallLlvmGcov       = os.path.join    ( self.pathCoverageDir, 'llvm-gcov.sh')
        self.pathLcov               = os.path.join    ( self.pathCoverageDir, 'lcov')
        self.pathCovInfo            = os.path.join    ( self.coverageTmpPath, 'cov.info')
        self.pathGenHtml            = os.path.join    ( self.pathCoverageDir, 'genhtml')


        if self.notRunExecutable == 'false' :
            self.run_executable()
    
        self.coppy_gcda_gcno_files()


    def run_executable(self) :
        pathExecutExt = get_exe( self.pathExecut )

        os.chdir( self.pathExecutDir )
        sub_process = subprocess.Popen([pathExecutExt], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

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


    def coppy_gcda_gcno_files( self ):

        if not os.path.isdir( self.coverageTmpPath ):
            os.makedirs(self.coverageTmpPath) 

        if self.buildConfig:
            pathConfigSegment = os.path.join(  '{0}.build'.format(self.executName), self.buildConfig )   

        #coppy '.gcda','.gcno' files
        listCoverData = []
        for rootdir, dirs, files in os.walk( self.pathBuild ):
            if rootdir.find( self.coverageTmpPath ) == -1 :
                if not self.buildConfig or rootdir.find( pathConfigSegment ) != -1:
                    for file in files:   
                        if file.endswith( ('.gcda','.gcno')  ): 
                            listCoverData += [os.path.join(rootdir, file)]
        
        for file in listCoverData:
            baseName = os.path.basename( file )
            pathOutFile = os.path.join(self.coverageTmpPath, baseName)

            if os.path.exists(pathOutFile):
                os.remove(pathOutFile)

            shutil.copy(file, self.coverageTmpPath )


    def generate_report_html( self ):

        if os.path.isdir( self.pathReportOut ):
            shutil.rmtree( self.pathReportOut )
        else:
            os.makedirs(self.pathReportOut)      

        os.chdir( self.pathCoverageDir ) 

        #
        params = [ self.pathLcov,
                    '--directory',      self.coverageTmpPath,  
                    '--base-directory', self.pathExecutDir,
                    '--gcov-tool',      self.pathCallLlvmGcov,
                    '--capture',   
                    '-o', self.pathCovInfo
                 ]
        
        subprocess.call(params)

        #
        params = [ self.pathGenHtml,
                   self.pathCovInfo, 
                   '-o', self.pathReportOut
                 ]

        subprocess.call(params)

    def generate_report_coverage( self ):

        eState = enum( UNDEF      =0, 
                       FIND_File  =1, 
                       FIND_Lines =2, 
                       FIND_Taken =3 )

        if not os.path.isfile(self.coverFilePath) or not os.access(self.coverFilePath, os.R_OK):
            print 'ERROR : file {0} is missing or is not readable'.format( self.coverFilePath )
            return

        coverFile      = open(self.coverFilePath).read()
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

        os.chdir( self.coverageTmpPath );
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
                    for rootdir, dirs, files in os.walk( self.coverageTmpPath ):
                        for file in files:   
                            if file.endswith( ('.gcda')  ): 
                                fileGcdaList += [os.path.join(rootdir, file)]

                for fileGcda in fileGcdaList:

                    params = [ self.pathLlvmCov, 'gcov',
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
                                fileCover.coverLines = float(re.findall("\d+\.\d+", split_line[1] )[0])
                                state = eState.FIND_File
                                subProcessContinue = False
                                subProcess.kill()

                        except IOError as err:
                            sys.stdout.write(err.message)
                            sys.stdout.flush()

        for test in testsCoverage:
            for fileCover in testsCoverage[ test ]:
                if CoverageMinimum > fileCover.coverLines:                
                    basename = os.path.basename( fileCover.file )
                    print '{0}:1: error: bad cover test {1} in file {2}: {3}% must be at least: {4}%'.format(fileCover.file,test,basename,fileCover.coverLines,CoverageMinimum)
            print ''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument( '--pathBuild', required = True )
    parser.add_argument( '--pathExecut', required = True )
    parser.add_argument( '--pathReportOut', required = True )
    parser.add_argument( '--buildConfig', choices=['Debug', 'Release'] )
    parser.add_argument( '--notRunExecutable', default = 'false', choices=['true', 'false'] )

    options = parser.parse_args()

    cov = CoverageReport( options  )

    cov.generate_report_html()
    cov.generate_report_coverage()


if __name__ == '__main__':
    main()


















