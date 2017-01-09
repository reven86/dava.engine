cmake_minimum_required (VERSION 3.0)
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set( NAME_MODULE TArcControls )                  # set name to plugin 
set( MODULE_TYPE PLUGIN )                        # set module type [INLINE, STATIC, PLUGIN]

set(TArcControlsRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set(HPP_FILES_RECURSE Sources/PluginSrc/*.h Sources/PluginSrc/*.hpp Sources/TArcControls/*.qml)
set(CPP_FILES_RECURSE Sources/PluginSrc/*.cpp)
set(QT_RES_FILES "Sources/Resources/*.qrc")

set(FIND_PACKAGE QT5)
set(EXCLUDE_FROM_ALL 1)

set( PLUGIN_PREFIX_MACOS "lib")
set( PLUGIN_DEBUG_POSTFIX_WIN "d")
set( PLUGIN_DEBUG_POSTFIX_MACOS "_debug")
set( PLUGIN_RELATIVE_PATH_TO_FOLDER "TArcControls")
file(GLOB QML_FILES "Sources/TArcControls/*")
set( PLUGIN_COPY_ADD_FILES ${QML_FILES})

set( PLUGIN_PRE_BUILD_COMMAND "python" "${CMAKE_CURRENT_LIST_DIR}/Sources/BuildScripts/pre-build.py")
set( PLUGIN_PRE_BUILD_PARAMS "${CMAKE_CURRENT_LIST_DIR}/Sources")

setup_main_module()
