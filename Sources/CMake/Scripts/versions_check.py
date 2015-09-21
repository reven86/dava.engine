#!/usr/bin/env python
import sys
import os
import subprocess
import argparse

parser = argparse.ArgumentParser()

parser.add_argument( 'path_prj' )
parser.add_argument( 'cur_vers' )
parser.add_argument( 'dir_list', nargs='+' )

args     = parser.parse_args()
dir_list = ' '.join(args.dir_list)

current_dir       = os.path.dirname(os.path.realpath(__file__)) + '/'
CURRENT_VERSIONS  = str(os.popen( 'python ' + current_dir + 'file_tree_hash.py ' + dir_list ).read())  
LAST_VERSIONS     = str(args.cur_vers)    
CURRENT_VERSIONS  = CURRENT_VERSIONS.rstrip(os.linesep)

print 'VersionsCheck ------ '
print 'CURRENT_VERSIONS ', CURRENT_VERSIONS
print 'LAST_VERSIONS    ', LAST_VERSIONS


if ( CURRENT_VERSIONS != LAST_VERSIONS ) :
    print 'Update cmake project !!!!', args.path_prj 
    os.popen( 'cmake ' + args.path_prj )


