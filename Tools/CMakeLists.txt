cmake_minimum_required( VERSION 3.0)

macro ( set_deploy_dir TARGET_NAME )
    if( MACOS )
        set( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app )

    else()
        set( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/${TARGET_NAME} )

    endif()    

endmacro ()

project               ( Megasolution  )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../Sources/CMake/Modules/" ) 

include               ( CMake-common )

find_package          ( DavaFramework REQUIRED )

add_subdirectory      ( "${DAVA_ROOT_DIR}/Sources/Tools"        ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

set                   ( IGNORE_FILE_TREE_CHECK 1 )#tmp
set                   ( TEXTURE_TOOL_FOUND 1 )
set                   ( DAVA_MEGASOLUTION  1 )

set                   ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set                   ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set                   ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set_deploy_dir        ( ResourceEditor )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor" ${CMAKE_CURRENT_BINARY_DIR}/ResourceEditor )

set_deploy_dir        ( QuickEd )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/QuickEd"        ${CMAKE_CURRENT_BINARY_DIR}/QuickEd )

set_deploy_dir        ( UIEditor )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/UIEditor"       ${CMAKE_CURRENT_BINARY_DIR}/UIEditor )

set_deploy_dir        ( UnitTests )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Projects/UnitTests"   ${CMAKE_CURRENT_BINARY_DIR}/UnitTests )

set_deploy_dir        ( SceneViewer )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Projects/SceneViewer"   ${CMAKE_CURRENT_BINARY_DIR}/SceneViewer )



