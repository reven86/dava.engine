cmake_minimum_required (VERSION 3.0)
project( DavaPluginTemplate) # to change the project name

set                   ( DAVA_USE_QT_WIDGETS true )

include               ( CMake-common )

find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )
find_package          ( NGT REQUIRED)

list(APPEND TARGET_LIBRARIES  core_generic_plugin core_qt_common core_ui_framework)

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/DavaTools/QtTools )
include_directories   ( "Classes" )
include_directories   ( "Classes/bullet" )
include_directories   ( "${DAVA_ROOT_DIR}/Sources/Tools")

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
-DNGT_INTEGRATION
)

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

set               ( Generated ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set               ( UiList ${UI_LIST} )

define_source_folders          ( SRC_ROOT "Classes" GLOBAL_ERASE_FOLDER )
generate_source_groups_project ( SRC_ROOT "Classes" "DataQt" Generated UiList)

set( APP_DATA               ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_PLIST        ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( ADDED_SRC           ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( DEPLOY_DIR          ${TOOL_OUTPUT_DIR})
set( QML_SCAN_DIR ${NGT_PLUGINS_PATH})

set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

SET_PROPERTY( TARGET DavaFramework  PROPERTY FOLDER "DavaCore")
SET_PROPERTY( TARGET DavaTools  PROPERTY FOLDER "DavaCore")
SET_PROPERTY( TARGET QtTools  PROPERTY FOLDER "DavaCore")

setup_main_dynlib()

SET_PROPERTY( TARGET ${PROJECT_NAME}  PROPERTY FOLDER "ToolsPlugin")
BW_GET_EXECUTABLE_DST_DIR(EXE_DIR)
BW_SET_BINARY_DIR(${PROJECT_NAME} "${EXE_DIR}/plugins")
