cmake_minimum_required (VERSION 2.8)

project      ( UnitTestsLib )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include      ( CMake-common )
include      ( FindDavaFramework )

include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

if( ANDROID )
    set( PLATFORM_SPECIFIC_FOLDER ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

elseif( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
    file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/*.xib" )

elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    
endif()

set( TeamcityOutput          ${DAVA_TOOLS_DIR}/TeamcityOutput  )
set( PackerTools             ${DAVA_TOOLS_DIR}/TexturePacker )
set( PackerToolsSCR          ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.h 
                             ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.cpp )

define_source_folders(       SRC_ROOT "Classes" ${TeamcityOutput}  ${PLATFORM_SPECIFIC_FOLDER} 
                             GLOB_ERASE_FOLDERS "_build"  )

generate_source_groups_project(  ROOT_DIR "Classes" ${PLATFORM_SPECIFIC_FOLDER}   
                                 GROUP_FOLDERS TeamcityOutput PackerTools PLATFORM_SPECIFIC_FOLDER )

set( MACOS_PLIST                 )
set( MACOS_ICO                   )
set( MACOS_DATA                  ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_DYLIB                 )

set( ANDROID_PACKAGE             "com.dava.unittests" )
set( ANDROID_APP_NAME            "UnitTestApp" )
set( ANDROID_ACTIVITY_APP_NAME   "UnitTests"   )

set( ANDROID_JAVA_SRC            ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/Java ${CMAKE_CURRENT_LIST_DIR}/android/src )
set( ANDROID_JAVA_LIBS           ${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/jar )
set( ANDROID_JAVA_RES            ${CMAKE_CURRENT_LIST_DIR}/android/res )
set( ANDROID_JAVA_ASSET          ${CMAKE_CURRENT_LIST_DIR}/Data ) 
set( ANDROID_JAVA_ASSET_FOLDER   "Data" )
set( ANDROID_ICO                 ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                   ${PackerToolsSCR} )
set( LIBRARIES                   )
set( LIBRARIES_RELEASE           )
set( LIBRARIES_DEBUG             )
set( ADDED_BINARY_DIR            )
set( EXECUTABLE_FLAG             WIN32 )
set( NOENTRY_FLAG                1 )
set( DISABLE_DUBUG               1 )

setup_main_executable()