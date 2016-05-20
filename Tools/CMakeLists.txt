cmake_minimum_required (VERSION 3.0)

macro ( set_deploy_dir TARGET_NAME )
    if( MACOS )
        set( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app )

    else()
        set( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/${TARGET_NAME} )

    endif()    

endmacro ()

project               ( Megasolution  )

set                   ( DAVA_USE_QT_WIDGETS true )
set                   ( DAVA_USE_RENDERSTATS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../Sources/CMake/Modules/" ) 

include               ( CMake-common )
include               ( NGTMacro )

#configure_ngt(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/app)

set                   ( DAVA_DISABLE_AUTOTESTS true )
find_package          ( DavaFramework REQUIRED COMPONENTS ALL )
find_package          ( DavaTools     REQUIRED )

set                   ( IGNORE_FILE_TREE_CHECK 1 )#tmp
set                   ( DAVA_MEGASOLUTION      1 )

set                   ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set                   ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set                   ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set_deploy_dir        ( ResourceEditor )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor" ${CMAKE_CURRENT_BINARY_DIR}/ResourceEditor )

set_deploy_dir        ( QuickEd )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/QuickEd"        ${CMAKE_CURRENT_BINARY_DIR}/QuickEd )

set_deploy_dir        ( UnitTests )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Projects/UnitTests"   ${CMAKE_CURRENT_BINARY_DIR}/UnitTests )

set_deploy_dir        ( TestBed)
add_subdirectory      ( "${DAVA_ROOT_DIR}/Projects/TestBed"     ${CMAKE_CURRENT_BINARY_DIR}/TestBed )

set_deploy_dir        ( SceneViewer )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Projects/SceneViewer"   ${CMAKE_CURRENT_BINARY_DIR}/SceneViewer )

set_deploy_dir        ( ResourcePacker )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/ResourcePacker" ${CMAKE_CURRENT_BINARY_DIR}/ResourcePacker )

set_deploy_dir        ( ImageUnpacker )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/ImageUnpacker" ${CMAKE_CURRENT_BINARY_DIR}/ImageUnpacker )

set_deploy_dir        ( AssetCacheClient )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/AssetCacheClient" ${CMAKE_CURRENT_BINARY_DIR}/AssetCacheClient )

set_deploy_dir        ( AssetCacheServer )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/AssetCacheServer" ${CMAKE_CURRENT_BINARY_DIR}/AssetCacheServer )

if ( WIN32 )
    set_deploy_dir    ( UWPRunner )
    add_subdirectory  ( "${DAVA_ROOT_DIR}/Tools/UWPRunner"   ${CMAKE_CURRENT_BINARY_DIR}/UWPRunner )
endif ()
