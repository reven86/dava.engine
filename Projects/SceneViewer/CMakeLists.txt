cmake_minimum_required (VERSION 3.0)
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include      ( CMake-common )
include      ( FindDavaFramework )

if( ANDROID )
    project ( SceneViewer )

else()
    project ( SceneViewer )

endif()

include_directories   ( "Classes" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

if( ANDROID )
    set( PLATFORM_SPECIFIC_FOLDER ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )

elseif( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )
    
elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    set( EXECUTABLE_FLAG WIN32 )

endif()

file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/*.xib" )

define_source_folders         ( SRC_ROOT "Classes" ${PLATFORM_SPECIFIC_FOLDER}  )
generate_source_groups_project( SRC_ROOT "Classes" )
add_subdirectory( ${DAVA_TOOLS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

set( WIN32_DATA                 ${CMAKE_CURRENT_LIST_DIR}/Data )

set( IOS_PLISTT                 ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/SceneVieweriPhone-Info.plist )
set( IOS_XIB                    ${XIB_LIST} )
set( IOS_DATA                   Data )

set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/SceneViewerMacOS-Info.plist )
set( MACOS_XIB                  ${XIB_LIST} )

set( MACOS_ICO                  )
set( MACOS_DATA                 ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_DYLIB                )

set( ANDROID_PACKAGE            "com.dava.unittests" )
set( ANDROID_APP_NAME           "UnitTestApp" )
set( ANDROID_ACTIVITY_APP_NAME  "UnitTests"   )

set( ANDROID_JAVA_SRC           ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/Java ${CMAKE_CURRENT_LIST_DIR}/android/src )
set( ANDROID_JAVA_LIBS          ${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/jar )
set( ANDROID_JAVA_RES           ${CMAKE_CURRENT_LIST_DIR}/android/res )
set( ANDROID_JAVA_ASSET         ${CMAKE_CURRENT_LIST_DIR}/Data ) 
set( ANDROID_ICO                ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                  ${IOS_ADD_SRC} )
set( LIBRARIES                  DavaTools )
set( LIBRARIES_RELEASE          )
set( LIBRARIES_DEBUG            )
set( ADDED_BINARY_DIR           )

setup_main_executable()