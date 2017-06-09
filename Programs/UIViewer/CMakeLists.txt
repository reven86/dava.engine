cmake_minimum_required (VERSION 3.0)
project ( UIViewer )

set          ( WARNINGS_AS_ERRORS true )
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include      ( CMake-common )

set ( DAVA_COREV2 1 )

dava_add_definitions ( -D__DAVAENGINE_COREV2__ )

find_package( DavaFramework REQUIRED COMPONENTS DAVA_DISABLE_AUTOTESTS "Sound" )

include_directories   ( "Classes" )

if( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )
    
elseif( WIN32 )
    set( EXECUTABLE_FLAG WIN32 )

endif()

file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/*.xib" )

define_source ( SOURCE "Classes" ${PLATFORM_SPECIFIC_FOLDER} )

set( MIX_APP_DATA                 "Data = ${DAVA_ROOT_DIR}/Programs/Data" 
                                  "Data = ${CMAKE_CURRENT_LIST_DIR}/Data" )

set( IOS_PLISTT                   ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UIVieweriPhone-Info.plist )

set( MACOS_PLIST                  ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UIViewerMacOS-Info.plist )
set( MACOS_XIB                    ${XIB_LIST} )

set( ANDROID_USE_STANDART_TEMLATE true )
set( ANDROID_PACKAGE              "com.dava.uiviewer" )
set( ANDROID_APP_NAME             "UIViewerApp" )
set( ANDROID_ACTIVITY_APP_NAME    "UIViewer"   )

set( ANDROID_ICO                   ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                     ${IOS_ADD_SRC} )

if (ANDROID)
    # Libraries and classes to load at startup
    set (ANDROID_BOOT_MODULES "crystax;c++_shared;fmodex;fmodevent;UIViewer")
    set (ANDROID_BOOT_CLASSES "com.dava.uiviewer.UIViewer")
endif()

if ( WINDOWS_UAP )
    set ( WIN_STORE_MANIFEST_PACKAGE_GUID "C3B05534-90DF-4E75-829E-25D10C77FE49" )
endif ()

setup_main_executable()

convert_graphics()
