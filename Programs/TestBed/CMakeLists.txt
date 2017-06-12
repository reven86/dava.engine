cmake_minimum_required (VERSION 3.0)
project ( TestBed )

# Enable MemoryManagerTest in UnitTests
# this variable should be defined in all dependant projects (dava framework, etc)
if( NOT DAVA_MEGASOLUTION )
    #set ( DAVA_MEMORY_PROFILER 1 )
endif()

set          ( WARNINGS_AS_ERRORS true )
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

# Now testbed is core v2 only
set (DAVA_COREV2 1)
dava_add_definitions(-D__DAVAENGINE_COREV2__)

# Uncomment to use local resources for android
# dava_add_definitions(-DUSE_LOCAL_RESOURCES)

set ( DAVA_COMPONENTS "Sound" "SampleModule" "DAVA_DISABLE_AUTOTESTS" )
if ( WIN32 AND NOT WIN_UAP AND NOT DAVA_PLATFORM_QT)
    list ( APPEND DAVA_COMPONENTS "CEFWebview" )
endif ()

# add physics
if(ANDROID AND ANDROID_ABI STREQUAL "x86" )
    # we don't have physx for android x86, so skip this
elseif (WINDOWS_UAP AND WINDOWS_UAP_PLATFORMS MATCHES "ARM")
    # we don't have physx for win uap arm, so skip this
else()
    list ( APPEND DAVA_COMPONENTS "Physics" )
    list ( APPEND DAVA_COMPONENTS "PhysicsDebug" )
endif()

if (DAVA_PLATFORM_QT)
    list(APPEND DAVA_COMPONENTS DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME )
    dava_add_definitions(-D__DAVAENGINE_QT__)
    if ( MACOS )
        dava_add_definitions(-DDISABLE_NATIVE_TEXTFIELD)
        dava_add_definitions(-DDISABLE_NATIVE_WEBVIEW)
        dava_add_definitions(-DDISABLE_NATIVE_MOVIEVIEW)
    endif()
    find_package(QT5 REQUIRED Core Gui Widgets QuickWidgets Network)
    set( ADDED_BINARY_DIR ${QT_ACTUAL_PATH}/bin )
endif()

dava_add_definitions(-DLOCALIZATION_DEBUG)


if( NOT WINDOWS_UAP AND ( MACOS OR WIN32 ) )
    add_plugin( SamplePlugin "${DAVA_MODULES_DIR}/SamplePlugin" COMPONENTS  ${DAVA_COMPONENTS} )
endif()


find_package( DavaFramework REQUIRED COMPONENTS ${DAVA_COMPONENTS} )
find_package( EmbeddedWebServer REQUIRED )


include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )

if( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )
    set( IOS_ADD_SRC ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TestBed.entitlements )
elseif( WIN32 )
    if (NOT DAVA_PLATFORM_QT)
        set( EXECUTABLE_FLAG WIN32 )
    endif()
endif()

file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/*.xib" )

define_source ( SOURCE    "Classes" )

set( MIX_APP_DATA                 "Data = ${DAVA_ROOT_DIR}/Programs/Data" 
                                  "Data = ${CMAKE_CURRENT_LIST_DIR}/Data" )

if (MACOS OR WIN32)
    set( APP_DATA  ${APP_DATA} "scripts")
endif()

set( IOS_PLISTT                 ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TestBed-Info.plist )

set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/Info.plist )
set( MACOS_XIB                  ${XIB_LIST} )

set( ANDROID_USE_STANDART_TEMLATE true )
set( ANDROID_PACKAGE            "com.dava.testbed" )
set( ANDROID_APP_NAME           "TestBedApp" )
set( ANDROID_ACTIVITY_APP_NAME  "TestBed"   )

set( ANDROID_ICO                ${CMAKE_CURRENT_LIST_DIR}/android/ic_launcher-web.png )

set( ADDED_SRC                  ${IOS_ADD_SRC} )
set( LIBRARIES                  )

if ( WINDOWS_UAP )
    set ( WIN_STORE_MANIFEST_PACKAGE_GUID "5B2576F2-B514-432B-BD8B-0665D4F96BC2" )
endif ()

if (ANDROID)
    # Libraries and classes to load at startup
    set (ANDROID_BOOT_MODULES "crystax;c++_shared;fmodex;fmodevent;TestBed")
    set (ANDROID_BOOT_CLASSES "com.dava.testbed.TestBed")
endif()

setup_main_executable()
convert_graphics()


if (IOS)
    # Termporal workaround for unit tests with memory profiling enabled
    # Reason: on iOS on some circumstances memory deallocating operation bypasses memory manager
    set_xcode_property( ${PROJECT_NAME} STRIP_INSTALLED_PRODUCT NO )
endif()
