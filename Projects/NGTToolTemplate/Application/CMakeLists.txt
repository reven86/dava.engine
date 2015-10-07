cmake_minimum_required (VERSION 3.0)
project( DavaToolHead )

include               ( CMake-common )

find_package(NGT REQUIRED)

list(APPEND TARGET_LIBRARIES core_generic_plugin_manager)

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)

define_source_folders          ( SRC_ROOT "." GLOBAL_ERASE_FOLDER )
list(APPEND PROJECT_SOURCE_FILES "plugins.txt")

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

setup_main_executable()

set_binary_dir( ${PROJECT_NAME} ${BIN_OUTPUT_DIR} "")

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
  ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/plugins.txt" "${BIN_OUTPUT_DIR}/${OUTPUTCONFIG}/plugins"
  )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
