cmake_minimum_required (VERSION 2.8)
project ( ColladaLibXML )
                          
define_source_folders  ( GLOB_ERASE_FOLDERS "include"  )

include_directories  ( "." )
include_directories  ( "include" )

add_library ( ColladaLibXML STATIC ${PROJECT_SOURCE_FILES} )






