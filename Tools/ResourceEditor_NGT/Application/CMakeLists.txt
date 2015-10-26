cmake_minimum_required (VERSION 3.0)
project( ${BW_BUNDLE_NAME} )

find_package(QT5 REQUIRED COMPONENTS Core)
find_package(NGT REQUIRED)

# setup executable project

list(APPEND TARGET_LIBRARIES core_generic_plugin_manager)

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )
set( ADDED_SRC main.cpp plugins.txt)

# for backward compatibility executable will not deploy qt. It must be done by DAVA plugin
set( NOT_DEPLOY_QT 1)

append_deploy_dependency(${PROJECT_NAME})
setup_main_executable()

BW_GET_EXECUTABLE_DST_DIR(EXE_DIR)
BW_SET_BINARY_DIR(${PROJECT_NAME} ${EXE_DIR})

if (WIN32)
  set(PLUGINS_FOLDER "${TOOL_OUTPUT_DIR}/plugins")
elseif (MACOS)
  set(PLUGINS_FOLDER "${TOOL_OUTPUT_DIR}/${PROJECT_NAME}.app/Contents/PlugIns/plugins")
endif()

ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGINS_FOLDER}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/plugins.txt"
            "${PLUGINS_FOLDER}")
