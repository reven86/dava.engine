cmake_minimum_required (VERSION 3.0)
if (COVERAGE_CHECK)
  set( COVERAGE true )
  set( COVERAGE_ARGS "selftest")
endif()
project ( TemplateTArc )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set (DAVA_MEMORY_PROFILER 1)

add_definitions(
    -D__DAVAENGINE_COREV2__
    -D__DAVAENGINE_QT__
    -DDISABLE_NATIVE_TEXTFIELD
    -DDISABLE_NATIVE_WEBVIEW
    -DDISABLE_NATIVE_MOVIEVIEW)

set ( DAVA_COREV2 1 )
set ( DAVA_PLATFORM_QT 1 )
set ( DAVA_COMPONENTS "Sound" "TArc")
set ( DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME 1)

find_package(DavaFramework REQUIRED "${DAVA_COMPONENTS}")
find_package(QT5 REQUIRED Core Gui Widgets QuickWidgets Network)

set( ADDED_BINARY_DIR ${QT_ACTUAL_PATH}/bin )

include_directories   ( "Classes" )

if( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
elseif( WIN32 )
    set( EXECUTABLE_FLAG WIN32 )
else()
    message(FATAL_ERROR "TArc supports only Win and Mac")
endif()

file(GLOB RCC_LIST "Resources/*.qrc")
file(GLOB QML_LIST "Resources/*.qml")
qt5_add_resources(RCC_APP_SRCS  ${RCC_LIST})
set(ADDED_SRC ${RCC_APP_SRCS})
set(QML_SCAN_DIR ${CMAKE_CURRENT_LIST_DIR})

define_source (SOURCE "Classes" ${PLATFORM_SPECIFIC_FOLDER} "Resources")

set(APP_DATA    ${CMAKE_CURRENT_LIST_DIR}/Data)
set(MACOS_PLIST ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist)

setup_main_executable()

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /ENTRY:wWinMainCRTStartup")
endif()
