cmake_minimum_required( VERSION 3.0 )

project               ( AssetCacheClient )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include               ( CMake-common )

find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )

include_directories   ( "Classes" )

define_source_folders         ( SRC_ROOT "Classes" )
generate_source_groups_project( SRC_ROOT "Classes" )


set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )

set( APP_DATA                    )
set( LIBRARIES                  )

setup_main_executable()