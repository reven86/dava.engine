cmake_minimum_required (VERSION 3.0)
project               ( NGTToolTemplate ) # to change the project name

cmake_policy(SET CMP0043 NEW)

set                   ( FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${FRAMEWORK_PATH}/Sources/CMake/Modules/" )
set                   ( USE_DYNAMIC_CRT 1)
set                   ( TARGET_PLUGIN_APP_FOLDER_NAME ${PROJECT_NAME})
set                   ( TOOL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Solution folders: http://athile.net/library/blog/?p=288
SET_PROPERTY( GLOBAL PROPERTY USE_FOLDERS ON )

include (CMake-common)
include (NGTMacro)

define_ngt_pathes("${FRAMEWORK_PATH}Sources/External/ngt/")

list(APPEND NGT_LIBS
      wg_types
      core_command_system
      core_common
      core_data_model
      core_generic_plugin
      core_dependency_system
      core_generic_plugin_manager
      core_logging
      core_logging_system
      core_qt_common
      core_qt_script
      core_reflection
      core_reflection_utils
      core_serialization
      core_string_utils
      core_ui_framework
      core_variant
      )

list(APPEND NGT_PLUGINS
      plg_alert_ui
      plg_reflection
      plg_variant
      plg_command_system
      plg_copy_paste
      plg_serialization
      plg_file_system
      plg_qt_common
      plg_editor_interaction
      plg_history_ui
      plg_qt_app)

list( APPEND QT_COMPONENTS
                        Core
                        Gui
                        Widgets
                        Concurrent
                        Qml
                        Network
                        Quick
                        QuickWidgets
                        UiTools
                        Xml)

add_definitions(
      -DDAVA_MEMORY_MANAGEMENT
)

find_package(QT5 REQUIRED COMPONENTS ${QT_COMPONENTS})
find_package(NGT REQUIRED)

BW_SET_USE_CONFIGURATION_NAME(1)
BW_SET_TEST_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_ASSET_PIPELINE_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_EXECUTABLE_DST_DIR(${TOOL_OUTPUT_DIR})

foreach(NGT_LIB ${NGT_LIBS})
    append_ngt_lib(${NGT_LIB})
endforeach()

foreach(NGT_PLUGIN ${NGT_PLUGINS})
    append_ngt_plugin(${NGT_PLUGIN})
endforeach()

cmake_policy(SET CMP0043 NEW)

set(CMAKE_DEBUG_POSTFIX _d)

add_subdirectory ( Application )
add_subdirectory ( Plugins/PluginTemplate )
