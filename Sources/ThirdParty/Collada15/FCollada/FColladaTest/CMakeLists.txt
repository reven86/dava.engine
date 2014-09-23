cmake_minimum_required (VERSION 2.8)
project ( FColladaTest )
                         
define_source_folders  ( GLOB_ERASE_FOLDERS "Samples" "RunFColladaTests" )

include_directories  ( "." )
include_directories  ( "../" )
include_directories  ("../LibXML/include")
                 
add_library ( FColladaTest STATIC ${PROJECT_SOURCE_FILES} )


                            



