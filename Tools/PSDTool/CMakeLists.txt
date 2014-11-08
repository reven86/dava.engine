cmake_minimum_required (VERSION 2.8)

project( PSDTool )

set          ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set          ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set          ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include      ( CMake-common         )

include_directories (.)
include_directories ( ${CMAKE_CURRENT_LIST_DIR}/../IMagickHelperLib/ )
include_directories ( ${DAVA_INCLUDE_DIR} ) 

add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../IMagickHelperLib" ${CMAKE_CURRENT_BINARY_DIR}/IMagickHelperLib )
add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries( ${PROJECT_NAME} ${DAVA_LIBRARY} ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/IMagickHelper.lib )

