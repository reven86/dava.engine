cmake_minimum_required (VERSION 3.0)
project ( TemplateTArc )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set (DAVA_MEMORY_PROFILER 1)

add_definitions(
    -D__DAVAENGINE_COREV2__
    -D__DAVAENGINE_QT__
    -DDISABLE_NATIVE_TEXTFIELD
    -DDISABLE_NATIVE_WEBVIEW)

set ( DAVA_COREV2 1)
set ( DAVA_COMPONENTS "Sound" "TArc")
set ( DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME 1)

find_package(QT5 REQUIRED Core Gui Widgets QuickWidgets Network)
find_package( DavaFramework REQUIRED "${DAVA_COMPONENTS}" )

set( ADDED_BINARY_DIR ${QT_ACTUAL_PATH}/bin )

include_directories   ( "Classes" )

if( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "WinSpecific")
    set( EXECUTABLE_FLAG WIN32 )
else()
    message(FATAL_ERROR "TArc supports only Win and Mac")
endif()

define_source ( SOURCE    "Classes" ${PLATFORM_SPECIFIC_FOLDER} )

set( APP_DATA                       ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )
set( WIN32_RESOURCES     ${CMAKE_CURRENT_LIST_DIR}/WinSpecific/Resources.rc )

setup_main_executable()
convert_graphics()

if (DAVA_COREV2)
    if (WIN32)
        set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /ENTRY:wWinMainCRTStartup")
    endif()
endif()
