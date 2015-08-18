cmake_minimum_required (VERSION 3.0)

project      ( DavaPak )
 
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include      ( CMake-common )

find_package( DavaFramework REQUIRED )

include_directories (Classes)
include_directories ( ${DAVA_INCLUDE_DIR} ) 

#add_definitions       (-D )

define_source_folders          ( SRC_ROOT "Classes" )
generate_source_groups_project ( SRC_ROOT "Classes" )

setup_main_executable()

if (WIN32)
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE" )    
endif ()