cmake_minimum_required (VERSION 3.0)
project( ResourceEditor_NGT )

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

get_filename_component( FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${FRAMEWORK_PATH}/Sources/CMake/Modules/" )
set                   ( USE_DYNAMIC_CRT 1)
set                   ( TARGET_PLUGIN_APP_FOLDER_NAME ${PROJECT_NAME})
set                   ( TOOL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")

if (DEPLOY_DIR)
      set (TOOL_OUTPUT_DIR ${DEPLOY_DIR})
endif()

# Solution folders: http://athile.net/library/blog/?p=288
SET_PROPERTY( GLOBAL PROPERTY USE_FOLDERS ON )

include (CMake-common)
include (NGTMacro)

# setup NGT framework

define_ngt_pathes("${FRAMEWORK_PATH}/Sources/External/ngt/")
get_ngt_modules(NGT_LIBS NGT_PLUGINS QT_COMPONENTS)

add_definitions(
      -DDAVA_MEMORY_MANAGEMENT
)

find_package(QT5 REQUIRED COMPONENTS ${QT_COMPONENTS})

set ( CMAKE_INCLUDE_CURRENT_DIR OFF )
set ( CMAKE_AUTOMOC OFF )

find_package(NGT REQUIRED)

if (NOT DEPLOY_DIR)
      BW_SET_USE_CONFIGURATION_NAME(1)
endif()
BW_SET_TEST_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_ASSET_PIPELINE_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_EXECUTABLE_DST_DIR(${TOOL_OUTPUT_DIR})

# NGT build system use this variable on Mac to resolve path where plugins must be copyed
set( BW_BUNDLE_NAME ${PROJECT_NAME})

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
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PLUGINS_FOLDER}")
ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/plugins.txt"
            "${PLUGINS_FOLDER}")

# append NGT targets to solution
foreach(NGT_LIB ${NGT_LIBS})
    append_ngt_lib(${NGT_LIB})
endforeach()

foreach(NGT_PLUGIN ${NGT_PLUGINS})
    append_ngt_plugin(${NGT_PLUGIN})
endforeach()

# append RE plugin to solution
set (BUILD_RE_PLUGIN 1)
list(APPEND EXCLUDE_LIBS "FColladaVS2010.lib" "libdxt.lib" "libdxtd.lib")
list(APPEND ADDITIONAL_RELEASE_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/FColladaVS2010.lib")
list(APPEND ADDITIONAL_DEBUG_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/D/FColladaVS2010.lib")
add_subdirectory(${FRAMEWORK_PATH}/Tools/ResourceEditor "ResourceEditor")
