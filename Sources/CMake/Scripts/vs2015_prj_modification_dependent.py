#!/usr/bin/env python
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import fromstring, ElementTree, Element

print '!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
if len(sys.argv) < 2:
    print 'Usage: Please enter path to vs project and name target'
    exit(1)

arg1  = sys.argv[1]#'<full_path>/WotBlitz.vcxproj' 
arg2  = sys.argv[2]#'WotBlitzLib.vcxproj'
print arg1
print arg2
ET.register_namespace("","http://schemas.microsoft.com/developer/msbuild/2003")
tree = ET.parse( arg1 )
root = tree.getroot()

ns = '{http://schemas.microsoft.com/developer/msbuild/2003}' 

modified_project = 'false'

for itgroup in root.iter( ns+'ItemDefinitionGroup' ):
    for childPrjRef in itgroup.iter( ns+'ProjectReference' ):
        linkLib = childPrjRef.findall( ns+ 'LinkLibraryDependencies') 
        print 'find'
        if( linkLib ):
            print linkLib[0].text
            linkLib[0].text = 'true'
            modified_project = 'true'
            print linkLib[0].text

if( modified_project == 'true' ) :
    tree.write( arg1 )
    print 'findwrite'

