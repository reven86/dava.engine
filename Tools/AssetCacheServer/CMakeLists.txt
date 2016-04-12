cmake_minimum_required (VERSION 3.0)
project               ( AssetCacheServer )

set                   ( DAVA_USE_QT_WIDGETS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include               ( CMake-common )

find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/DavaTools/QtTools )

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)
include_directories   ( "Classes" )

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

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

set               ( Generated ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set               ( UiList ${UI_LIST} )

define_source_folders          ( SRC_ROOT "Classes" GLOBAL_ERASE_FOLDER )
generate_source_groups_project ( SRC_ROOT "Classes" "DataQt" Generated UiList)

#set( APP_DATA             ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( ADDED_SRC            ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

set( DISABLE_SOUNDS         true)

setup_main_executable()
