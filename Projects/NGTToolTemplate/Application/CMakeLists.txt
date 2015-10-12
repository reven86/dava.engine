cmake_minimum_required (VERSION 3.0)
project( DavaToolHead )

include               ( CMake-common )
set( BW_BUNDLE_NAME ${PROJECT_NAME} PARENT_SCOPE)

find_package(NGT REQUIRED)

list(APPEND TARGET_LIBRARIES core_generic_plugin_manager)

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)

define_source_folders          ( SRC_ROOT "." GLOBAL_ERASE_FOLDER )
list(APPEND PROJECT_SOURCE_FILES "plugins.txt")

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )
set( NOT_DEPLOY_QT 1)

setup_main_executable()

BW_GET_EXECUTABLE_DST_DIR(EXE_DIR)
BW_SET_BINARY_DIR(${PROJECT_NAME} ${EXE_DIR})

if (WIN32)
  set(PLUGINS_FOLDER "${TOOL_OUTPUT_DIR}/${CMAKE_CFG_INTDIR}/plugins")
elseif (MACOS)
  set(PLUGINS_FOLDER "${TOOL_OUTPUT_DIR}/${CMAKE_CFG_INTDIR}/${PROJECT_NAME}.app/Contents/PlugIns/plugins")
endif()

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PLUGINS_FOLDER}")
ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/plugins.txt"
            "${PLUGINS_FOLDER}")
