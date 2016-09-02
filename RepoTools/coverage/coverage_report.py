#!/usr/bin/env python
import sys
import os
import fnmatch
import argparse
import subprocess
import shutil
import json
import re
import time
import io

from string import Template
from collections import namedtuple
import HTMLParser
from HTMLParser import HTMLParser

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

def retrieve_name(var):
    import inspect
    callers_local_vars = inspect.currentframe().f_back.f_locals.items()
    return [var_name for var_name, var_val in callers_local_vars if var_val is var][0]

def configure_file( file_path_template, file_path_out, values_string_list, values_obj ) :
    dicts = {}

    for item in values_string_list :
        item_value  = '' 
        vl_name = retrieve_name(values_obj)
        if hasattr( values_obj, item ) :
            exec("%s = %s" % ('item_value', '{0}.{1}'.format(vl_name,item))) 

        dicts.update( { item : item_value } )  

    with open( file_path_template ) as file_template, \
         open( file_path_out, 'w' ) as file_generated :  
            template = Template( file_template.read() )
            file_generated.write( template.safe_substitute( dicts ) )

class CheckTimeDependence():
    def __init__(self, pathExecut, timeFile ):

        self.pathExecut = pathExecut
        self.timeFile   = timeFile

    def create_time_file( self ):

        dirTimeFile = os.path.dirname( self.timeFile ) 
        if not os.path.isdir( dirTimeFile ):
            os.makedirs( dirTimeFile ) 
        with open(self.timeFile, "w") as  file:
            file.write( time.ctime(os.path.getmtime(self.pathExecut)) )

    def is_updated( self ):

        timeExecute     = time.ctime(os.path.getmtime(self.pathExecut))
        timeExecuteOld  = ''

        if os.path.isfile( self.timeFile ) :
            with open(self.timeFile) as f:
                timeExecuteOld = f.readline()

        return timeExecute != timeExecuteOld


class CoverageReport():

    def __init__(self, arg ):

        self.arg                    = arg
        self.pathBuild              = arg.pathBuild
        self.pathExecut             = arg.pathExecut
        self.pathReportOut          = arg.pathReportOut
        self.pathReportOutFull      = os.path.join( arg.pathReportOut, 'CoverageFull' ) 
        self.pathReportOutTests     = os.path.join( arg.pathReportOut, 'CoverageTests' )

        self.buildConfig            = arg.buildConfig
        self.notExecute             = arg.notExecute
        self.teamcityMode           = arg.teamcityMode

        self.coverageTmpPath        = os.path.join( arg.pathBuild, 'CoverageTmpData' )
    
        self.pathExecutDir          = os.path.dirname ( self.pathExecut )
        self.executName             = os.path.basename( self.pathExecut )
        self.executName, ExecutExt  = os.path.splitext( self.executName )
        
        self.pathCoverageDir        = os.path.dirname (os.path.realpath(__file__))
        self.pathExecutTime         = os.path.join( self.pathBuild, 'CMakeFiles/{0}.time'.format( self.executName ) )


        self.tfExec                 = CheckTimeDependence( self.pathExecut, self.pathExecutTime )


        self.coverFilePath          = os.path.join    ( self.pathExecutDir,   '{0}.cover'.format( self.executName ) )
        self.pathLlvmCov            = os.path.join    ( self.pathCoverageDir, 'llvm-cov')
        self.pathLlvmProfdata       = os.path.join    ( self.pathCoverageDir, 'llvm-profdata')
        self.pathCallLlvmGcov       = os.path.join    ( self.pathCoverageDir, 'llvm-gcov.sh')
        self.pathLcov               = os.path.join    ( self.pathCoverageDir, 'lcov')
        self.pathCovInfoFull        = os.path.join    ( self.coverageTmpPath, 'cov_full.info')
        self.pathCovInfoTests       = os.path.join    ( self.coverageTmpPath, 'cov_tests.info')        
        self.pathGenHtml            = os.path.join    ( self.pathCoverageDir, 'genhtml')

        self.pathMixHtml            = os.path.join    ( self.pathReportOut,   'index.html')
        self.pathMixHtmlTemplate    = os.path.join    ( self.pathCoverageDir, 'mix_index_html.in')

        self.pathUnityPack          = ''
        self.testsCoverage          = {}
        self.testsCoverageFiles     = []

        self.mixHtmlValueStrList    = { 'full_title', 'full_date', 'full_linesHit', 'full_linesTotal', 'full_linesCoverage',
                                        'full_funcHit', 'full_funcTotal', 'full_funcCoverage', 'full_coverLegendCovLo', 'full_coverLegendCovMed',
                                        'full_coverLegendCovHi', 'local_title', 'local_date', 'local_linesHit', 'local_linesTotal', 'local_linesCoverage',
                                        'local_funcHit', 'local_funcTotal', 'local_funcCoverage', 'local_coverLegendCovLo', 'local_coverLegendCovMed',
                                        'local_coverLegendCovHi' }

        if self.notExecute == 'false' and self.tfExec.is_updated() == True:
            self.__clear_old_gcda()
            self.tfExec.create_time_file()
            pathExecutExt = get_exe( self.pathExecut )
            os.chdir( self.pathExecutDir )
            self.__execute( [ pathExecutExt ] )
    
        self.__processing_gcda_gcno_files()
        self.__load_json_cover_data()

    def __clear_old_gcda( self ):        
        for rootdir, dirs, files in os.walk( self.pathBuild ):
            for file in files:   
                if file.endswith( ('.gcda')  ):
                    os.remove( os.path.join(rootdir, file) ) 

    def __build_print( self, str ):        
        sys.stdout.write("{0}\n".format(str) )
        sys.stdout.flush()

    
    def __teamcity_print( self, str ):
        if self.teamcityMode == 'true' :
            __build_print( self, str )

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
                for ext in [ 'cpp', 'c' ]:
                    find_list = find_files( '{0}.{1}'.format( file, ext ) , projectFolders )
                    if len( find_list ):
                        fileCover = FileCover( find_list[0], None )
                        self.testsCoverage.setdefault(test, []).append( fileCover )
                        self.testsCoverageFiles +=  [find_list[0]]

    def __processing_gcda_gcno_files( self ):
        
        if os.path.isdir( self.coverageTmpPath ):
            shutil.rmtree( self.coverageTmpPath )
                
        os.makedirs( self.coverageTmpPath ) 

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

    def generate_mix_html( self ):
        from HTMLParser import HTMLParser 
        class ValueList: pass
        vl = ValueList() 

        vl.full_title = 'QQWEQWERQEWRQEWRQWER'

        configure_file( self.pathMixHtmlTemplate, self.pathMixHtml, self.mixHtmlValueStrList, vl )


    def generate_report_html( self ):

        self.__teamcity_print( '##teamcity[testStarted name=\'Generate cover html\']' )

        if os.path.isdir( self.pathReportOut ):
            shutil.rmtree( self.pathReportOut )
        else:
            os.makedirs(self.pathReportOut)      

        ###
        params = [ self.pathLcov,
                    '--directory',      self.coverageTmpPath,  
                    '--base-directory', self.pathExecutDir,
                    '--gcov-tool',      self.pathCallLlvmGcov,
                    '--capture',   
                    '-o', self.pathCovInfoFull
                 ]        
        self.__build_print( params )                 
        self.__execute( params )
        
        ###        
        params = [ self.pathLcov,
                    '--extract', self.pathCovInfoFull, 
                    '-o', self.pathCovInfoTests
                 ] + self.testsCoverageFiles
        self.__build_print( params )         
        self.__execute( params ) 

        ###
        params = [ self.pathGenHtml,
                   self.pathCovInfoFull, 
                   '-o', self.pathReportOutFull,
                   '--legend'
                 ]
        self.__build_print( params )                 
        self.__execute( params) 

        params = [ self.pathGenHtml,
                   self.pathCovInfoTests, 
                   '-o', self.pathReportOutTests,
                   '--legend'
                 ]
        self.__build_print( params )                 
        self.__execute( params)         
        ###

        self.__teamcity_print( '##teamcity[testFinished name=\'Generate cover html\']' )
        
        self.generate_mix_html()


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
                self.__build_print( "Processing file: {0}".format( fileName ) )

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
                    self.__build_print( '{0}:1: error: bad cover test {1} in file {2}: {3}% must be at least: {4}%'.format(fileCover.file,test,basename,fileCover.coverLines,CoverageMinimum) )
            self.__build_print( '' )
        
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
    parser.add_argument( '--notExecute', default = 'false', choices=['true', 'false'] ) 
    parser.add_argument( '--teamcityMode', default = 'false', choices=['true', 'false'] )    
    parser.add_argument( '--buildMode', default = 'false', choices=['true', 'false'] )
    parser.add_argument( '--runMode', default = 'false', choices=['true', 'false'] )

    options = parser.parse_args()

    cov = CoverageReport( options )

    if options.buildMode == 'true' :
        cov.generate_report_coverage()

    elif options.runMode == 'true' :
        cov.generate_report_html()

    else:
        cov.generate_report_html()
        cov.generate_report_coverage() 

if __name__ == '__main__':
    main()


















