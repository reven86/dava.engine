#!/usr/bin/env python
import sys
import os
import fnmatch
import argparse
import subprocess
import shutil
import json
import re

CoverageMinimum    = 75.0

class FileCover():
    def __init__(self, fileName, coverLines):
        self.file = fileName
        self.coverLines = coverLines

def enum(**enums):
    return type('Enum', (), enums)

def find_files(pattern, dirs):
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

def check_sting_in_file( str, fname ):
    with open(fname) as dataf:
        return any(str in line for line in dataf)


class CoverageReport():

    def __init__(self, arg ):

        self.arg                    = arg
        self.pathBuild              = arg.pathBuild
        self.pathExecut             = arg.pathExecut
        self.pathReportOut          = arg.pathReportOut
        self.pathReportOutFull      = os.path.join( arg.pathReportOut, 'CoverageFull' ) 
        self.pathReportOutTests     = os.path.join( arg.pathReportOut, 'CoverageTests' )

        self.buildConfig            = arg.buildConfig
        self.teamcityMode           = arg.teamcityMode

        self.coverageTmpPath        = os.path.join( arg.pathBuild, 'CoverageTmpData' )
    
        self.pathExecutDir          = os.path.dirname ( self.pathExecut )
        self.executName             = os.path.basename( self.pathExecut )
        self.executName, ExecutExt  = os.path.splitext( self.executName )
        
        self.pathCoverageDir        = os.path.dirname (os.path.realpath(__file__))

        self.coverFilePath          = os.path.join    ( self.pathExecutDir,   '{0}.cover'.format( self.executName ) )
        self.pathLlvmCov            = os.path.join    ( self.pathCoverageDir, 'llvm-cov')
        self.pathLlvmProfdata       = os.path.join    ( self.pathCoverageDir, 'llvm-profdata')
        self.pathCallLlvmGcov       = os.path.join    ( self.pathCoverageDir, 'llvm-gcov.sh')
        self.pathLcov               = os.path.join    ( self.pathCoverageDir, 'lcov')
        self.pathCovInfoFull        = os.path.join    ( self.coverageTmpPath, 'cov_full.info')
        self.pathCovInfoTests       = os.path.join    ( self.coverageTmpPath, 'cov_tests.info')        
        self.pathGenHtml            = os.path.join    ( self.pathCoverageDir, 'genhtml')

        self.pathUnityPack          = ''
        self.testsCoverage          = {}
        self.testsCoverageFiles     = []

        if self.teamcityMode == 'false' :
            pathExecutExt = get_exe( self.pathExecut )
            os.chdir( self.pathExecutDir )
            self.__execute( [ pathExecutExt ] )
    
        self.__coppy_gcda_gcno_files()
        self.__load_json_cover_data()

    def __teamcity_print( self, str ):
        if self.teamcityMode == 'true' :
            print str

    def __execute(self, param) :

        sub_process = subprocess.Popen(param, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

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

    def __load_json_cover_data( self ):
        if not os.path.isfile(self.coverFilePath) or not os.access(self.coverFilePath, os.R_OK):
            print 'ERROR : file {0} is missing or is not readable'.format( self.coverFilePath )
            return

        coverFile          = open(self.coverFilePath).read()
        jsonData           = json.loads(coverFile)
        projectFolders     = jsonData[ 'ProjectFolders' ].split(' ')

        self.pathUnityPack      = jsonData[ 'UnityFolder' ]
        self.testsCoverage      = {}
        self.testsCoverageFiles = []

        for test in jsonData[ 'Coverage' ]:
            testedFiles = jsonData[ 'Coverage' ][ test ].split(' ')
            for file in testedFiles:
                for ext in [ 'cpp', 'c']:
                    find_list = find_files( '{0}.{1}'.format( file, ext ) , projectFolders )
                    if len( find_list ):
                        fileCover = FileCover( find_list[0], None )
                        self.testsCoverage.setdefault(test, []).append( fileCover )
                        self.testsCoverageFiles +=  [find_list[0]]

    def __coppy_gcda_gcno_files( self ):

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

    def __error_log_coverage_file( self, test, file ):

        os.chdir( self.pathExecutDir );

        pathExecutExt = get_exe( self.pathExecut )
        param = [ self.pathLlvmCov, 
                  'show', 
                  pathExecutExt, 
                  '-instr-profile={0}.profdata'.format(self.executName), 
                  file  
                ] 
        sub_process = subprocess.Popen(param, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        subProcessContinue = True
        while subProcessContinue:
            try:
                line = sub_process.stdout.readline()
                if line == '':
                    subProcessContinue = False
                    continue

                split_line = line.split( )
                if len( split_line ) == 0:
                    continue
                
                coverValue = re.findall('\d+', split_line[0])
                if len( coverValue ) > 0 and int(coverValue[0]) == 0:
                    lineValue = re.findall('\d+', split_line[1])
                    print '{0}:{1}: warning: bad cover: {2}'.format(file,int(lineValue[0]),test)

            except IOError as err:
                sys.stdout.write(err.message)
                sys.stdout.flush()

    def __find_unity_pack_gcda( self, file ):

        if len( file ) == 0:
            return []

        unity_files = []
        for root, dirs, files in os.walk( self.pathUnityPack ):
            for name in files:
                unity_file = os.path.join(root, name)
                if( check_sting_in_file(file, unity_file) ) :
                    fileName = os.path.basename( unity_file )
                    fileName, fileExt = os.path.splitext( fileName )
                    return [ '{0}.gcda'.format(fileName) ]
        return []

    def generate_report_html( self ):

        self.__teamcity_print( '##teamcity[testStarted name=\'Generate cover html\']' )

        if os.path.isdir( self.pathReportOut ):
            shutil.rmtree( self.pathReportOut )
        else:
            os.makedirs(self.pathReportOut)      

        os.chdir( self.pathCoverageDir ) 

        ###
        params = [ self.pathLcov,
                    '--directory',      self.coverageTmpPath,  
                    '--base-directory', self.pathExecutDir,
                    '--gcov-tool',      self.pathCallLlvmGcov,
                    '--capture',   
                    '-o', self.pathCovInfoFull
                 ]        
        self.__execute( params )
        
        ###        
        params = [ self.pathLcov,
                    '--extract', self.pathCovInfoFull, 
                    '-o', self.pathCovInfoTests
                 ] + self.testsCoverageFiles
        print params       
         
        self.__execute( params ) 

        ###
        params = [ self.pathGenHtml,
                   self.pathCovInfoFull, 
                   '-o', self.pathReportOutFull,
                   '--legend'
                 ]
        self.__execute( params) 

        params = [ self.pathGenHtml,
                   self.pathCovInfoTests, 
                   '-o', self.pathReportOutTests,
                   '--legend'
                 ]
        self.__execute( params)         
        ###

        self.__teamcity_print( '##teamcity[testFinished name=\'Generate cover html\']' )


    def generate_report_coverage( self ):

        self.__teamcity_print( '##teamcity[testStarted name=\'Coverage test\']' )

        eState = enum( UNDEF      =0, 
                       FIND_File  =1, 
                       FIND_Lines =2, 
                       FIND_Taken =3 )


        self.__execute( [ self.pathLlvmProfdata, 'merge', '-o', '{0}.profdata'.format(self.executName), 'default.profraw' ] )

        os.chdir( self.coverageTmpPath )
        for test in self.testsCoverage:
            state = eState.FIND_File

            for fileCover in self.testsCoverage[ test ]:
                fileName          = os.path.basename( fileCover.file )
                fileName, fileExt = os.path.splitext( fileName )

                fileGcda     = '{0}.gcda'.format(fileName)
                fileGcdaList = [ ]

                if  os.path.isfile( fileGcda ) :
                    fileGcdaList = [ fileGcda ]
                else:
                    fileGcdaList = self.__find_unity_pack_gcda( fileCover.file )

                if len( fileGcdaList ) == 0:
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
        
        for test in self.testsCoverage:
            for fileCover in self.testsCoverage[ test ]:
                if CoverageMinimum > fileCover.coverLines:                
                    basename = os.path.basename( fileCover.file )
                    print '{0}:1: error: bad cover test {1} in file {2}: {3}% must be at least: {4}%'.format(fileCover.file,test,basename,fileCover.coverLines,CoverageMinimum)
            print ''
        
        if self.teamcityMode == 'false' :
            for test in self.testsCoverage:
                for fileCover in self.testsCoverage[ test ]:
                    if CoverageMinimum > fileCover.coverLines:                
                        basename = os.path.basename( fileCover.file )
                        self.__error_log_coverage_file( test, fileCover.file )
        else:
            for test in self.testsCoverage:
                for fileCover in self.testsCoverage[ test ]:
                    if CoverageMinimum > fileCover.coverLines:                
                        basename = os.path.basename( fileCover.file )
                        self.__teamcity_print( '##teamcity[testFailed name=\'Cover\' message=\'{0}\' details=\'file {1}: {2}% must be at least: {3}%\']'.format(test,basename,fileCover.coverLines,CoverageMinimum) )

        self.__teamcity_print( '##teamcity[testFinished name=\'Coverage test\']' )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument( '--pathBuild', required = True )
    parser.add_argument( '--pathExecut', required = True )
    parser.add_argument( '--pathReportOut', required = True )
    parser.add_argument( '--buildConfig', choices=['Debug', 'Release'] )
    parser.add_argument( '--teamcityMode', default = 'false', choices=['true', 'false'] )

    options = parser.parse_args()

    cov = CoverageReport( options )

    if options.teamcityMode == 'true' :
        cov.generate_report_html()
    cov.generate_report_coverage()


if __name__ == '__main__':
    main()


















