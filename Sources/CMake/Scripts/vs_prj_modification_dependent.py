#!/usr/bin/env python
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import fromstring, ElementTree, Element

if len(sys.argv) < 2:
    print 'Usage: Please enter path to vs project and name target'
    exit(1)

arg1  = sys.argv[1]#'<full_path>/WotBlitz.vcxproj' 
arg2  = sys.argv[2]#'WotBlitzLib.vcxproj'

ET.register_namespace("","http://schemas.microsoft.com/developer/msbuild/2003")
tree = ET.parse( arg1 )
root = tree.getroot()

ns = '{http://schemas.microsoft.com/developer/msbuild/2003}' 

modified_project = 'false'

for neighbor in root.iter( ns+'ItemGroup' ):
    for child in neighbor:
        ret = child.attrib.get( 'Include' ).find( arg2 ) 
        if( ret !=  -1 ) :

            find_val = child.findall( ns+'LinkLibraryDependencies' ) 
            if( find_val ) :
                if( find_val[0].text == 'false' ) :
                    modified_project = 'true'
                find_val[0].text = 'true'
            else :
                node = Element( 'LinkLibraryDependencies' )  
                node.text = 'true'
                child.append( node  )
                modified_project = 'true'
                  
            find_val = child.findall( ns+'UseLibraryDependencyInputs' ) 
            if( find_val  ) :
                if( find_val[0].text == 'false' ) :
                    modified_project = 'true'
                find_val[0].text = 'true'
            else :
                modified_project = 'true'
                node = Element( 'UseLibraryDependencyInputs' )  
                node.text = 'true'
                child.append( node  )

if( modified_project == 'true' ) :
    tree.write( arg1 )

