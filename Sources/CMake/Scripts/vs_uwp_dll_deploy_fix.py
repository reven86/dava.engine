#!/usr/bin/env python
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import fromstring, ElementTree, Element

if len(sys.argv) != 2:
    print 'Usage: Please enter path to vs project'
    exit(1)

full_path_to_proj = sys.argv[1]

ET.register_namespace("","http://schemas.microsoft.com/developer/msbuild/2003")
tree = ET.parse( full_path_to_proj )
root = tree.getroot()

ns = '{http://schemas.microsoft.com/developer/msbuild/2003}' 

modified_project = False

for neighbor in root.iter( ns+'ItemGroup' ):
    for child in neighbor:
        #search included dll
        if( child.tag == ns+'None' and child.attrib.get( 'Include' ).find( 'dll' ) != -1 ):
            #find out target config of library
            words = child.attrib.get( 'Include' ).split('\\')
            words.reverse()
            config = (words[1] + '|' + words[2]).lower()
            
            #mark dll as deployment content only for target config
            for content in child.iter( ns+'DeploymentContent' ):
                condition = content.attrib.get( 'Condition' ).lower()
                if ( condition.find(config) != -1 ):
                    if ( content.text != 'true' ):
                        content.text = 'true'
                        modified_project = True
                else:
                    if ( content.tag != ns+'ExcludedFromBuild' or content.text != 'true' ):
                        content.tag = ns+'ExcludedFromBuild'
                        content.text = 'true'
                        modified_project = True

if( modified_project == True ):
    tree.write( full_path_to_proj )