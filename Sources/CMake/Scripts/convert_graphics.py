import os;
import sys;
import os.path;
import pickle;
import zlib;
import string;
import sys;
import platform;
import shutil
import time;
import argparse;
from subprocess import call

def _str_to_bool(s):
    """Convert string to bool (in argparse context)."""
    if s.lower() not in ['true', 'false', '1', '0']:
        raise ValueError('Need bool; got %r' % s)
    return {'true': True, 'false': False, '1': True, '0': False}[s.lower()]

def add_boolean_argument(parser, name, default=False):                                                                                               
    """Add a boolean argument to an ArgumentParser instance."""
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        '--' + name, nargs='?', default=default, const=True, type=_str_to_bool)
    group.add_argument('--no' + name, dest=name, action='store_false')
    
def delete_folder(folder_path):
    if os.path.exists( folder_path ):
        print 'delete folder ', folder_path
        shutil.rmtree( folder_path )

def convert_graphics_args( args ):
    pathDava       = os.path.realpath( args.pathDava )
    pathDataSource = os.path.realpath( args.pathDataSource )
    pathTools      = os.path.realpath( os.path.join( args.pathDava, "Tools/Bin" ) )
    
    print ''
    print 'Convert graphics  '
    print 'Path to Dava Engine -> ', pathDava
    print 'Path to DataSource -> ', pathDataSource
    print 'Path to Dava tools ->', pathTools
    print 'Clear data ->', args.clearData

    paramPacker = ''
    if args.paramPacker :
        paramPacker = ''.join(args.paramPacker)
        paramPacker = paramPacker.replace(";", " ")
        print 'ResourcePacker parameters -> ', paramPacker
          
    if args.clearData == True:
        print ''
        delete_folder( os.path.join(pathDataSource, "$process")  )
        delete_folder( os.path.join(pathDataSource, "../Data/Gfx")  )
        print ''

    gfxDirs = filter(lambda x: x[0:3] == "Gfx", os.listdir(pathDataSource));
    
    startTime = time.time()  
    os.chdir( pathTools );    
    xxx_trash_bin_path = os.path.relpath( pathTools, pathDataSource )
    for dir in gfxDirs:
        pathGfx = os.path.join(pathDataSource, dir)
        params = ["./ResourcePacker", pathGfx, xxx_trash_bin_path, paramPacker ]
        print 'Call ->', params, '\n'
        rc = os.spawnv(os.P_WAIT, "./ResourcePacker", params);
             
    deltaTime = time.time() - startTime;
    
    print 'End convert graphics. Operation  time ->', deltaTime
    print ''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument( '--pathDava', required = True )
    parser.add_argument( '--pathDataSource', required = True )
    parser.add_argument('--paramPacker', nargs='*' )
    add_boolean_argument( parser, 'clearData', default=False )

    parser.set_defaults( func = convert_graphics_args )
    args = parser.parse_args()
    args.func( args )           


if __name__ == '__main__':
    main()



