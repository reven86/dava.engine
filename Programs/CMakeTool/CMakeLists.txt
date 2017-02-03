cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( CMakeTool )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include               ( CMake-common )

find_package          ( QT5 REQUIRED )
find_package          ( QtHelpers  REQUIRED )

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)
include_directories   ( "Classes" )

if ( WIN32 )
    set( LIBRARIES_RELEASE  "msvcrt.lib" )
    set( LIBRARIES_DEBUG    "msvcrtd.lib" )
endif()

file                          ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources             ( RCC_APP_SRCS  ${RCC_LIST} )

define_source ( SOURCE "Classes" )

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/icon.icns  )

set( ADDED_SRC            ${RCC_APP_SRCS}  CMakeTool.rc )

set( QML_SCAN_DIR               ${CMAKE_CURRENT_LIST_DIR} )
set( APP_DATA                   ${CMAKE_CURRENT_LIST_DIR}/Data )

ASSERT(QT_ACTUAL_PATH "Qt path was not resolved")
set( ADDED_BINARY_DIR     ${QT_ACTUAL_PATH}/bin )

setup_main_executable()
