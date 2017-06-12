cmake_minimum_required (VERSION 3.0)

set( COVERAGE true )

project ( UnitTests )

set          ( WARNINGS_AS_ERRORS true )
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common )

# Enable MemoryManagerTest in UnitTests
# this variable should be defined in all dependant projects (dava framework, etc)
if( NOT DAVA_MEGASOLUTION AND NOT DISABLE_MEMORY_PROFILER )
    set ( DAVA_MEMORY_PROFILER 1 )
endif()

if (LINUX)
    set ( DAVA_MEMORY_PROFILER 0 )
endif()

# Now unittests is core v2 only
set (DAVA_COREV2 1)
dava_add_definitions(-D__DAVAENGINE_COREV2__)

if (LINUX)
    dava_add_definitions(-DDISABLE_NATIVE_MOVIEVIEW)
    dava_add_definitions(-DDISABLE_NATIVE_TEXTFIELD)
    dava_add_definitions(-DDISABLE_NATIVE_WEBVIEW)
endif()

if ( WIN32 OR MACOS )
    find_package(DavaTools REQUIRED )
endif()

# Enable LOCALIZATION_DEBUG in UnitTests to verify successful compilation
dava_add_definitions(-DLOCALIZATION_DEBUG)

if (NOT LINUX)
    # Enable sound after fmod libraries
    set ( DAVA_COMPONENTS "Sound" "NetworkCore" DAVA_DISABLE_AUTOTESTS )
else()
    set ( DAVA_COMPONENTS "NetworkCore" DAVA_DISABLE_AUTOTESTS )
endif()

if ( WIN32 AND NOT WINDOWS_UAP )
    list ( APPEND DAVA_COMPONENTS "CEFWebview" )
endif ()

if (NOT ANDROID)
    list ( APPEND DAVA_COMPONENTS "SampleModule" )
endif()

# add physics
if(ANDROID AND ANDROID_ABI STREQUAL "x86" )
    # we don't have physx for android x86, so skip this
elseif (WINDOWS_UAP AND WINDOWS_UAP_PLATFORMS MATCHES "ARM")
    # we don't have physx for win uap arm, so skip this
else()
    dava_add_definitions(-D__DAVAENGINE_PHYSICS_ENABLED__)
    list ( APPEND DAVA_COMPONENTS "Physics" )
endif()

find_package( Steam REQUIRED )
find_package( DavaFramework REQUIRED COMPONENTS "${DAVA_COMPONENTS}" )
find_package( EmbeddedWebServer REQUIRED )

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

define_source                  ( SOURCE "Classes" )

set( STEAM_APPID                Win32Specific/steam_appid.txt )

set( MIX_APP_DATA                 "Data = ${DAVA_ROOT_DIR}/Programs/Data" 
                                  "Data = ${CMAKE_CURRENT_LIST_DIR}/Data" )

set( IOS_PLISTT                 ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/UnitTests-Info.plist )

set( MACOS_XIB                  ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/MainMenu.xib)
set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )

set( ANDROID_PACKAGE            "com.dava.unittests" )
set( ANDROID_APP_NAME           "UnitTestApp" )

set( ANDROID_ICO                ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                  ${IOS_ADD_SRC} )

#uncomment this 2 strings to link libjpeg as additional project.
#set( LIBRARIES jpeg )
#add_subdirectory ( "${CMAKE_CURRENT_LIST_DIR}/../../Libs/libjpeg" ${CMAKE_CURRENT_BINARY_DIR}/libjpeg )

if ( WINDOWS_UAP )
    set ( WIN_STORE_MANIFEST_PACKAGE_GUID "49484B77-9BB6-4FBC-9D56-77593EF55C45" )
endif ()

if (ANDROID)
    # Libraries and classes to load at startup
    set (ANDROID_BOOT_MODULES "crystax;c++_shared;fmodex;fmodevent;UnitTests")
    set (ANDROID_BOOT_CLASSES "com.dava.unittests.UnitTests")
endif()

exctact_external_unittests()
setup_main_executable()
convert_graphics()
 
if (IOS)
    # Termporal workaround for unit tests with memory profiling enabled
    # Reason: on iOS on some circumstances memory deallocating operation bypasses memory manager
    set_xcode_property( ${PROJECT_NAME} STRIP_INSTALLED_PRODUCT NO )
endif()
