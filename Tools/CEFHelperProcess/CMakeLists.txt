cmake_minimum_required (VERSION 3.0)
project ( CEFHelperProcess )

set    ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include( CMake-common )
find_package( DavaFramework REQUIRED )

if ( NOT ENABLE_CEF )
    message( FATAL_ERROR "Chromium Embedded Framework is not enabled" )
endif()

define_source_folders ( SRC_ROOT "Classes"  )
generate_source_groups_project( SRC_ROOT "Classes" )
setup_main_executable()

set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG   "/SUBSYSTEM:CONSOLE" )
set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE" )