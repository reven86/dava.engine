cmake_minimum_required (VERSION 3.0)

project      ( UnitTests )

if( ANDROID )
    project      ( UnitTestsLib )

endif()

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

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )
    set( IOS_ADD_SRC ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UnitTests.entitlements )

elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    set( EXECUTABLE_FLAG WIN32 )

endif()

add_subdirectory(               ${DAVA_TOOLS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

define_source_folders(          SRC_ROOT "Classes" ${PLATFORM_SPECIFIC_FOLDER} 
                                GLOB_ERASE_FOLDERS "_build"  )

generate_source_groups_project( ROOT_DIR "Classes" ${PLATFORM_SPECIFIC_FOLDER}   
                                GROUP_FOLDERS PLATFORM_SPECIFIC_FOLDER )

set( IOS_PLISTT                 ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UnitTests-Info.plist )
set( IOS_XIB                    ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainWindow.xib ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/iPad/MainWindow-iPad.xib )
set( IOS_DATA                    Data )

set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )
set( MACOS_XIB                  ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainMenu.xib )

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
set( ANDROID_JAVA_ASSET_FOLDER  "Data" )
set( ANDROID_ICO                ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                  ${PackerToolsSCR}  ${IOS_ADD_SRC} )
set( LIBRARIES                  TextureTool )
set( LIBRARIES_RELEASE          )
set( LIBRARIES_DEBUG            )
set( ADDED_BINARY_DIR           )

setup_main_executable()