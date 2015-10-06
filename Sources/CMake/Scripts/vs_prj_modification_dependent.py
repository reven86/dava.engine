#!/usr/bin/env python
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import fromstring, ElementTree, Element

if len(sys.argv) < 2:
    print 'Usage: Please enter path to vs project and name depend target project'
    exit(1)

full_path_to_proj = sys.argv[1]
name_depend_proj  = sys.argv[2]

ET.register_namespace("","http://schemas.microsoft.com/developer/msbuild/2003")
tree = ET.parse( full_path_to_proj )
root = tree.getroot()

ns = '{http://schemas.microsoft.com/developer/msbuild/2003}' 

modified_project = False

for neighbor in root.iter( ns+'ItemGroup' ):
    for child in neighbor:
        ret = child.attrib.get( 'Include' ).find( name_depend_proj ) 
        if( ret !=  -1 ) :

            find_val = child.findall( ns+'LinkLibraryDependencies' ) 
            if( find_val ) :
                if( find_val[0].text == 'false' ) :
                    modified_project = True
                find_val[0].text = 'true'
            else :
                node = Element( 'LinkLibraryDependencies' )  
                node.text = 'true'
                child.append( node  )
                modified_project = True
                  
            find_val = child.findall( ns+'UseLibraryDependencyInputs' ) 
            if( find_val  ) :
                if( find_val[0].text == 'false' ) :
                    modified_project = True
                find_val[0].text = 'true'
            else :
                modified_project = True
                node = Element( 'UseLibraryDependencyInputs' )  
                node.text = 'true'
                child.append( node  )

if( modified_project ) :
    tree.write( full_path_to_proj )

