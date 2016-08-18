cmake_minimum_required (VERSION 3.0)
project ( TestBed )

# Enable MemoryManagerTest in UnitTests
# this variable should be defined in all dependant projects (dava framework, etc)
if( NOT DAVA_MEGASOLUTION )
    #set ( DAVA_MEMORY_PROFILER 1 )
endif()

if (DAVA_COREV2)
    add_definitions(-D__DAVAENGINE_COREV2__)
    # Uncomment to use local resources for android
    # add_definitions(-DUSE_LOCAL_RESOURCES)
endif()

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set ( DAVA_COMPONENTS "Sound")
if ( WIN32 AND NOT WIN_UAP AND NOT DAVA_PLATFORM_QT)
    list ( APPEND DAVA_COMPONENTS "CEFWebview" )
endif ()

if (DAVA_COREV2 AND DAVA_PLATFORM_QT)
    set (DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME 1)
    add_definitions(-D__DAVAENGINE_QT__)
    if ( MACOS )
        add_definitions(-DDISABLE_NATIVE_TEXTFIELD)
        add_definitions(-DDISABLE_NATIVE_WEBVIEW)
        add_definitions(-DDISABLE_NATIVE_MOVIEVIEW)
    endif()
    find_package(QT5 REQUIRED Core Gui Widgets QuickWidgets Network)
    set( ADDED_BINARY_DIR ${QT_ACTUAL_PATH}/bin )
endif()

find_package( DavaFramework REQUIRED "${DAVA_COMPONENTS}" )

include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )

if( MACOS )
    set( PLATFORM_SPECIFIC_FOLDER "MacOSSpecific" )
elseif( IOS )
    set( PLATFORM_SPECIFIC_FOLDER "iOSSpecific" )
    set( IOS_ADD_SRC ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/TestBed.entitlements )

elseif( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    set( EXECUTABLE_FLAG WIN32 )
endif()

file ( GLOB_RECURSE XIB_LIST "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_SPECIFIC_FOLDER}/*.xib" )

define_source ( SOURCE    "Classes" ${PLATFORM_SPECIFIC_FOLDER} )

set( APP_DATA                   ${CMAKE_CURRENT_LIST_DIR}/Data )

if (MACOS)
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

setup_main_executable()
if (DAVA_COREV2)
    if (WIN32)
        set_property(TARGET TestBed APPEND_STRING PROPERTY LINK_FLAGS " /ENTRY:wWinMainCRTStartup")
    endif()
endif()

convert_graphics()

if (IOS)
    # Termporal workaround for unit tests with memory profiling enabled
    # Reason: on iOS on some circumstances memory deallocating operation bypasses memory manager
    set_xcode_property( ${PROJECT_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS YES )
    set_xcode_property( ${PROJECT_NAME} STRIP_INSTALLED_PRODUCT NO )
endif()
