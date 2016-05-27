cmake_minimum_required (VERSION 3.0)
project ( CEFHelperProcess )

set    ( DAVA_NO_RENDER true )
set    ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include( CMake-common )

find_package( DavaFramework REQUIRED )

include_directories   ( "Classes" )
define_source_folders ( SRC_ROOT "Classes" )
generate_source_groups_project( SRC_ROOT "Classes" )

list ( APPEND UNIFIED_IGNORE_LIST "Classes/main.cpp" "Classes/CEFDavaRenderApp.cpp" )

set( MAC_DISABLE_BUNDLE     true)
set( DISABLE_SOUNDS         true)

setup_main_executable()

if ( WIN32 )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG   "/SUBSYSTEM:WINDOWS" )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS" )
endif ()
