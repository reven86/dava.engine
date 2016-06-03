cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( CMakeTool )

get_filename_component( FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

set                   ( DAVA_DISABLE_AUTOTESTS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
set                   ( DAVA_USE_QT_WIDGETS    true )

include               ( CMake-common )


find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)
include_directories   ( "Classes" )

if ( WIN32 )
    set( LIBRARIES_RELEASE  "msvcrt.lib" )
    set( LIBRARIES_DEBUG    "msvcrtd.lib" )
endif()

file                          ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources             ( RCC_APP_SRCS  ${RCC_LIST} )

define_source_folders         ( SRC_ROOT "Classes" ERASE_FOLDERS "_build"    )

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/icon.icns  )

set( ADDED_SRC            ${RCC_APP_SRCS}  CMakeTool.rc )

set( QML_SCAN_DIR				${CMAKE_CURRENT_LIST_DIR} )
set( APP_DATA                   ${CMAKE_CURRENT_LIST_DIR}/Data )
set( ADDED_BINARY_DIR           ${QT5_PATH_WIN}/bin )

setup_main_executable()
