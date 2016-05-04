#!/usr/bin/env python
import sys
import os
import subprocess
import argparse
import platform

def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

def search_program(program):
    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        folders = os.environ["PATH"].split(os.pathsep)
        folders.append("/Applications/CMake.app/Contents/bin")
        for path in folders:
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if platform.system() == 'Windows':
                exe_file = exe_file + '.exe'
            if is_exe(exe_file):
                return exe_file

    return False

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

    cmake_program = search_program("cmake")
    if False == cmake_program:
        print "cmake command not found."
        exit()

    print 'Update cmake project !!!!', args.path_prj 

    call_string = [cmake_program, args.path_prj]
    subprocess.check_output(call_string)



