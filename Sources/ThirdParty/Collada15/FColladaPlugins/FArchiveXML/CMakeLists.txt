cmake_minimum_required (VERSION 2.8)
project ( FArchiveXML )
                          
define_source_folders  ( )
include_directories    ( "." )
include_directories    ( "${COLLADA_INCLUDES}" )
include_directories    ( "${LIBRARY_XML_INCLUDES}" )

add_library            ( FArchiveXML STATIC ${PROJECT_SOURCE_FILES} )





