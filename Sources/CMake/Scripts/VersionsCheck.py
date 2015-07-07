#!/usr/bin/env python
import sys
import os
import subprocess

arguments    = sys.argv[1:]

if len(arguments) < 3:
    print 'Usage: Please enter correct arguments'
    exit(1)

arg1 = sys.argv[1]
arg2 = sys.argv[2]
arg3 = sys.argv[3]
arg2 = arg2.replace(';', ' ')
arg2 = arg2.replace(',', ' ')

current_dir       = os.path.dirname(os.path.realpath(__file__)) + '/'
VERSIONS          = os.popen( 'python ' + current_dir + 'FileTreeHash.py ' + arg2 ).read()  
CURRENT_VERSIONS  = arg3    
VERSIONS          = VERSIONS.rstrip(os.linesep)

print 'VersionsCheck ------ '
print 'VERSIONS        ', VERSIONS
print 'CURRENT_VERSIONS', CURRENT_VERSIONS

if ( VERSIONS !=  CURRENT_VERSIONS ) :
    print 'Update cmake project !!!!', arg1 
    os.popen( 'cmake ' + arg1 )



