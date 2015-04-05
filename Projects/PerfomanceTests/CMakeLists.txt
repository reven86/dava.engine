cmake_minimum_required (VERSION 3.0)
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include      ( CMake-common )
include      ( FindDavaFramework )

project      ( PerformanceTests )

include_directories   ( "iOSSpecific" )
include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )
include_directories   ( "Classes/Controller" )
include_directories   ( "Classes/Screen" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

set( PackerTools             ${DAVA_TOOLS_DIR}/TexturePacker )
set( PackerToolsSCR          ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.h 
                             ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.cpp )

if( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
endif()
if( ANDROID )
    set( PLATFORM_SPECIFIC_FOLDER ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

elseif( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/*.xib" )

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )

elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    set( EXECUTABLE_FLAG WIN32 )

endif()

define_source_folders( SRC_ROOT "Classes" "iOSSpecific" ${PLATFORM_SPECIFIC_FOLDER} ${TeamcityOutput} )
generate_source_groups_project( )
add_subdirectory( ${DAVA_TOOLS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

set( APP_DATA             ${CMAKE_CURRENT_LIST_DIR}/Data )

set( MACOS_XIB            ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainMenu.xib )
set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/icon.icns  )

set( IOS_PLISTT           ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )
set( IOS_XIB              ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainWindow.xib ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/iPad/MainWindow-iPad.xib )

set( ANDROID_USE_STANDART_TEMLATE true )
set( ANDROID_PACKAGE            "com.dava.performancetests" )
set( ANDROID_APP_NAME           "PerformanceTestApp" )
set( ANDROID_ACTIVITY_APP_NAME  "PerformanceTests"   )

set( ADDED_SRC            ${IOS_ADD_SRC})
set( LIBRARIES            DavaTools )
set( LIBRARIES_RELEASE    )
set( LIBRARIES_DEBUG      )
set( ADDED_BINARY_DIR     )

setup_main_executable()