cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( CMakeTool )

set                   ( DAVA_DISABLE_AUTOTESTS true )
set                   ( DAVA_USE_QT_WIDGETS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include               ( CMake-common )

find_package          ( QT5           REQUIRED )


include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)
include_directories   ( "Classes" )


file                          ( GLOB RCC_LIST "*.qrc" )
qt5_add_resources             ( RCC_APP_SRCS  ${RCC_LIST} )

define_source_folders         ( SRC_ROOT "Classes" GLOB_ERASE_FOLDERS "_build"    )
generate_source_groups_project( SRC_ROOT "Classes" GROUP_FOLDERS Generated UiList )
 
set( APP_DATA                   ${CMAKE_CURRENT_LIST_DIR}/Data )
set( ADDED_SRC                  ${RCC_APP_SRCS} )
set( ADDED_BINARY_DIR           ${QT5_PATH_WIN}/bin )

setup_main_executable()
