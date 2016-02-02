cmake_minimum_required (VERSION 3.0)
project( ResourceEditorDLL )

include               ( CMake-common )

# append RE libs to solution
set (BUILD_AS_PLUGIN 1)
list(APPEND EXCLUDE_LIBS "FColladaVS2010.lib" "libdxt.lib" "libdxtd.lib")
list(APPEND ADDITIONAL_RELEASE_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/FColladaVS2010.lib")
list(APPEND ADDITIONAL_DEBUG_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/D/FColladaVS2010.lib")
add_subdirectory(${FRAMEWORK_PATH}/Tools/ResourceEditor "ResourceEditor")

find_package(NGT REQUIRED)

list(APPEND TARGET_LIBRARIES core_generic_plugin core_qt_common core_ui_framework)

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR})
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/ResourceEditor)
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/DavaTools/QtTools )
include_directories   ( "Classes" )
include_directories   ( "${DAVA_ROOT_DIR}/Sources/")
include_directories   ( "${DAVA_ROOT_DIR}/Sources/Tools")
include_directories   ( "${DAVA_ROOT_DIR}/Sources/Internal")
include_directories   ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor")
include_directories   ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor/Classes")
include_directories   ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor/Classes/bullet")
include_directories   ( "${DAVA_ROOT_DIR}/Tools/ResourceEditor/Classes/Qt")
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/include" )

add_definitions       (
-DQT_GUI_LIB
-DQT_NETWORK_LIB
-DQT_CORE_LIB
-DQT_HAVE_MMX
-DQT_HAVE_3DNOW
-DQT_HAVE_SSE
-DQT_HAVE_MMXEXT
-DQT_HAVE_SSE2
-DQT_THREAD_SUPPORT
-DDAVA_QT
)

if (WIN32)
  add_definitions(-D__DAVAENGINE_BEAST__)
endif()

define_source_folders          ( SRC_ROOT "Classes"  "Interfaces" GLOBAL_ERASE_FOLDER )
generate_source_groups_project ( SRC_ROOT "Classes"  "Interfaces" "DataQt")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(AUTOGEN_TARGETS_FOLDER "QtGenerated")

#link with ResourceEditor.lib
list(APPEND LIBRARIES "ResourceEditor")

set( APP_DATA               ${FRAMEWORK_PATH}/Tools/ResourceEditor/Data )
set( MACOS_DYLIB          ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib )
set( QML_SCAN_DIR ${NGT_PLUGINS_PATH})

append_deploy_dependency(${PROJECT_NAME})
setup_main_dynlib()

BW_GET_EXECUTABLE_DST_DIR(EXE_DIR)
BW_SET_BINARY_DIR(${PROJECT_NAME} "${EXE_DIR}/plugins")
if (MACOS)
  BW_COPY_PLUGIN_TO_BUNDLE(${PROJECT_NAME})
endif()
