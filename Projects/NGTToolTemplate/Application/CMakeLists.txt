cmake_minimum_required (VERSION 3.0)
project( DavaToolHead )

include               ( CMake-common )

find_package(NGT REQUIRED)

list(APPEND TARGET_LIBRARIES core_generic_plugin_manager)

set(CMAKE_CURRENT_BINARY_DIR ${TOOL_OUTPUT_DIR})
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)

define_source_folders          ( SRC_ROOT "." GLOBAL_ERASE_FOLDER )
list(APPEND PROJECT_SOURCE_FILES "plugins.txt")

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

setup_main_executable()

BW_GET_EXECUTABLE_DST_DIR(EXE_DIR)
BW_SET_BINARY_DIR(${PROJECT_NAME} ${EXE_DIR})

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            ${TOOL_OUTPUT_DIR}/${CMAKE_CFG_INTDIR}/plugins)
ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
             "${CMAKE_CURRENT_LIST_DIR}/plugins.txt"
             "${TOOL_OUTPUT_DIR}/${CMAKE_CFG_INTDIR}/plugins")
