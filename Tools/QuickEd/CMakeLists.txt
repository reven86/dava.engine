cmake_minimum_required (VERSION 3.0) 

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( QuickEd )

set                   ( DAVA_DISABLE_AUTOTESTS true )
set                   ( DAVA_USE_QT_WIDGETS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include               ( CMake-common )

add_definitions       ( -DDISABLE_NATIVE_WEBVIEW -DDISABLE_NATIVE_TEXTFIELD)

find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )


include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   (.)
include_directories   ( "Classes" )
include_directories   ( "Classes/Helpers" )
include_directories   ( "Classes/Controllers" )
include_directories   ( "Classes/Models" )

add_definitions       ( 
-DDAVA_QT
-DQT_HAVE_MMX
-DQT_HAVE_3DNOW
-DQT_HAVE_SSE
-DQT_HAVE_MMXEXT
-DQT_HAVE_SSE2
-DQT_THREAD_SUPPORT
)

file                          ( GLOB UI_LIST "Ui/*.ui" )
qt5_wrap_ui                   ( UI_APP_HDRS  ${UI_LIST} )

file                          ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources             ( RCC_APP_SRCS  ${RCC_LIST} )

set                           ( Generated    ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set                           ( UiList       ${UI_LIST} )


define_source_folders         ( SRC_ROOT "Classes" GLOB_ERASE_FOLDERS "_build"    )
generate_source_groups_project( SRC_ROOT "Classes" GROUP_FOLDERS Generated UiList )
 
set( APP_DATA                   ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO                  ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( WIN32_RESOURCES            ${CMAKE_CURRENT_LIST_DIR}/WinSpecific/Resources.rc )
set( MACOS_DYLIB                ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib )
set( ADDED_SRC                  ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( ADDED_BINARY_DIR           ${QT5_PATH_WIN}/bin )

setup_main_executable()
