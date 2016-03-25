cmake_minimum_required (VERSION 3.0)
project ( UnitTests )

set( DAVA_DISABLE_AUTOTESTS true )

# Enable MemoryManagerTest in UnitTests
# this variable should be defined in all dependant projects (dava framework, etc)
if( NOT DAVA_MEGASOLUTION AND NOT DISABLE_MEMORY_PROFILER )
    set ( DAVA_MEMORY_PROFILER 1 )
endif()

set          ( WARNINGS_AS_ERRORS true )
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common )

find_package( Steam REQUIRED )
find_package( DavaFramework REQUIRED )

include_directories   ( "Classes" )

if( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )

elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )
    set( IOS_ADD_SRC ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UnitTests.entitlements )

elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    set( EXECUTABLE_FLAG WIN32 )

endif()

define_source_folders         ( SRC_ROOT "Classes" ${PLATFORM_SPECIFIC_FOLDER}  )
generate_source_groups_project( SRC_ROOT "Classes" )

set( APP_DATA                   ${CMAKE_CURRENT_LIST_DIR}/Data )

set( IOS_PLISTT                 ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UnitTests-Info.plist )

set( MACOS_XIB                  ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainMenu.xib)
set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )

set( ANDROID_PACKAGE            "com.dava.unittests" )
set( ANDROID_APP_NAME           "UnitTestApp" )
set( ANDROID_ACTIVITY_APP_NAME  "UnitTests"   )

set( ANDROID_ICO                ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                  ${IOS_ADD_SRC} )

#uncomment this 2 strings to link libjpeg as additional project.
#set( LIBRARIES jpeg )
#add_subdirectory ( "${CMAKE_CURRENT_LIST_DIR}/../../Libs/libjpeg" ${CMAKE_CURRENT_BINARY_DIR}/libjpeg )

if ( WINDOWS_UAP )
    set ( WIN_STORE_MANIFEST_PACKAGE_GUID "49484B77-9BB6-4FBC-9D56-77593EF55C45" )
endif ()

setup_main_executable()

if (IOS)
    # Termporal workaround for unit tests with memory profiling enabled
    # Reason: on iOS on some circumstances memory deallocating operation bypasses memory manager
    set_xcode_property( ${PROJECT_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS YES )
    set_xcode_property( ${PROJECT_NAME} STRIP_INSTALLED_PRODUCT NO )
endif()
