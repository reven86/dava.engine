cmake_minimum_required (VERSION 3.0)

project      ( ResourceArchiver )

set (DAVA_COREV2 1)
add_definitions(-DDISABLE_NATIVE_WEBVIEW)
add_definitions(-DDISABLE_NATIVE_TEXTFIELD)
add_definitions(-DDISABLE_NATIVE_MOVIEVIEW)
add_definitions(-D__DAVAENGINE_COREV2__)
add_definitions(-DCONSOLE)
add_definitions(-D_CONSOLE)
 
set          ( WARNINGS_AS_ERRORS true )
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include      ( CMake-common )

find_package( DavaFramework REQUIRED )
find_package( DavaTools     REQUIRED )

include_directories (Classes)
include_directories ( ${DAVA_INCLUDE_DIR} ) 

define_source ( SOURCE  "Classes"  )

set( MAC_DISABLE_BUNDLE     true)
set( DISABLE_SOUNDS         true)

setup_main_executable()

set_subsystem_console()