#!/usr/bin/env python
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import fromstring, ElementTree, Element

if len(sys.argv) < 2:
    print 'Usage: Please enter path to vs project and name target'
    exit(1)

proj  = sys.argv[1]

ET.register_namespace("","http://schemas.microsoft.com/developer/msbuild/2003")
tree = ET.parse( proj )
root = tree.getroot()

ns = '{http://schemas.microsoft.com/developer/msbuild/2003}' 

modified_project = 'false'

for propertyGroup in root.iter( ns + 'PropertyGroup' ):
    tgtMinVersion = propertyGroup.find( ns + 'WindowsTargetPlatformMinVersion' )
    if tgtMinVersion is None:
        continue

    if tgtMinVersion.text == '10.0.10240.0':
        tgtMinVersion.text = '10.0.10166.0'
        modified_project = 'true'
        
if( modified_project == 'true' ) :
    tree.write( proj )