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
find_package(NGT REQUIRED)

if (NOT DEPLOY_DIR)
      BW_SET_USE_CONFIGURATION_NAME(1)
endif()
BW_SET_TEST_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_ASSET_PIPELINE_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_EXECUTABLE_DST_DIR(${TOOL_OUTPUT_DIR})

include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH} )

# NGT build system use this variable on Mac to resolve path where plugins must be copyed
set( BW_BUNDLE_NAME "NGT")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Application ${BW_BUNDLE_NAME})

# append NGT targets to solution
foreach(NGT_LIB ${NGT_LIBS})
    append_ngt_lib(${NGT_LIB})
endforeach()

foreach(NGT_PLUGIN ${NGT_PLUGINS})
    append_ngt_plugin(${NGT_PLUGIN})
endforeach()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ResourceEditorDLL ResourceEditorDLL)

set(ADDITIONAL_INCLUDES "${CMAKE_CURRENT_LIST_DIR}/ResourceEditorDLL")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/GraphEditor GraphEditor)
