cmake_minimum_required( VERSION 3.0)

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

set                   ( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/ResourceEditor )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor" ${CMAKE_CURRENT_BINARY_DIR}/ResourceEditor )

set                   ( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/QuickEd )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/QuickEd"        ${CMAKE_CURRENT_BINARY_DIR}/QuickEd )

set                   ( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/UIEditor )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/UIEditor"       ${CMAKE_CURRENT_BINARY_DIR}/UIEditor )

set                   ( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/UnitTests )
add_subdirectory      ( "${DAVA_ROOT_DIR}/Projects/UnitTests"   ${CMAKE_CURRENT_BINARY_DIR}/UnitTests )



