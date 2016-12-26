cmake_minimum_required (VERSION 3.0)
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set( NAME_MODULE TArcControls )                  # set name to plugin 
set( MODULE_TYPE PLUGIN )                        # set module type [INLINE, STATIC, PLUGIN]

set(TArcControlsRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set(HPP_FILES_RECURSE Sources/TArcControls/*.h Sources/TArcControls/*.hpp)
set(CPP_FILES_RECURSE Sources/TArcControls/*.cpp)
set(QT_RES_FILES "Sources/Resources/Resources.qrc")
set(QT_QML_FILES "Sources/Qml/*.qml")

set(FIND_PACKAGE QT5)
set(EXCLUDE_FROM_ALL 1)

set( PLUGIN_DEBUG_POSTFIX "d")
set( PLUGIN_OUT_DIR "d:/dev/dava.framework/Tools/ResourceEditor/_build/Debug/plugins/" )

set( PLUGIN_PRE_BUILD_COMMAND "python" "${CMAKE_CURRENT_LIST_DIR}/Sources/BuildScripts/pre-build.py")
set( PLUGIN_PRE_BUILD_PARAMS "${CMAKE_CURRENT_LIST_DIR}/Sources")

setup_main_module()
