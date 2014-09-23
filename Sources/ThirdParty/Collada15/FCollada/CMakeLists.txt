cmake_minimum_required (VERSION 2.8)
project ( FCollada )
                        
define_source_folders          ( GLOB_ERASE_FOLDERS "_build" "FColladaTest" "LibXML" )
generate_source_groups_project ()

include_directories            (.)
include_directories            ("LibXML/include")

add_subdirectory               ( "../FColladaPlugins/FArchiveXML" ${CMAKE_CURRENT_BINARY_DIR}/Plugins_FArchiveXML )

add_library                    ( FCollada STATIC ${PROJECT_SOURCE_FILES} )
target_link_libraries          ( FCollada FArchiveXML )  

install_libraries              ( FCollada )


