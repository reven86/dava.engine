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

for itemGroup in root.iter( ns + 'ItemGroup' ):
    tgtSDKReference = itemGroup.find( ns + 'SDKReference' )
    if tgtSDKReference is None:
        continue
    childs = itemGroup.getchildren()
    for itemSDK in childs:
        if itemSDK.get("Include") == "WindowsIoT, Version=10.0.10240.0":
            exit
    itemGroup.append(Element(tgtSDKReference.tag, {'Include': 'WindowsIoT, Version=10.0.10240.0'}))
    modified_project = 'true'

if( modified_project == 'false' ) :
    newGroup = Element( ns + 'ItemGroup' )
    print newGroup.tag
    newExtension = Element(ns + 'SDKReference', {'Include': 'WindowsIoT, Version=10.0.10240.0'})
    print newExtension.tag
    print newExtension.attrib
    newGroup.append(newExtension)
    root.append(newGroup)
    modified_project = 'true'
        
if( modified_project == 'true' ) :
    tree.write( proj )