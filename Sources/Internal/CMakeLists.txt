cmake_minimum_required (VERSION 2.8.6)

project( DavaFramework )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../CMake/Modules/" ) 

include( CMake-common )

set ( FRAMEWORK_DEFINITIONS 
-D_UNICODE 
-DUNICODE 
-D_CRT_SECURE_NO_DEPRECATE 
-D_CRT_NON_CONFORMING_SWPRINTFS
-DCURL_STATICLIB
)

if( NOT DISABLE_SOUNDS )
    list( APPEND FRAMEWORK_DEFINITIONS -DDAVA_FMOD )

endif()

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDAVA_DEBUG")

find_package(ZLIB )
if( ZLIB_FOUND )
    list (APPEND DAVA_THIRD_PARTY_INCLUDES_PATH ${ZLIB_INCLUDE_DIR} )
    list (APPEND LIST_SHARED_LIBRARIES          ${ZLIB_LIBRARY}     )
endif()

if( ANDROID )
    set ( LIBRARY_TYPE SHARED )
    
    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*" )
    list ( APPEND LIST_SHARED_LIBRARIES ${LIB_LIST}  )

    link_directories( "${DAVA_THIRD_PARTY_LIBRARIES_PATH}" )
    set ( LIST_LIBRARIES
        "libcrypto_android.a"
        "libEGL.so"
        "libGLESv1_CM.so"
        "liblog.so"
        "libandroid.so"
        "-lGLESv2"
        "-lstdc++" )
        
    list ( APPEND LIST_SHARED_LIBRARIES ${LIST_LIBRARIES} )

    set ( FRAMEWORK_DEFINITIONS  ${FRAMEWORK_DEFINITIONS} -D__ANDROID__ )
    set ( LIBRARY_OUTPUT_PATH    ${THIRD_PARTY_LIBRARIES_PATH} )

elseif( MACOS )
    set ( LIBRARY_TYPE STATIC )

    add_definitions( -D__DAVAENGINE_PTHREAD__ ) 

    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )

    foreach( NAME   Cocoa
                    GLUT
                    OpenGL
                    AppKit
                    AVFoundation
                    Security
                    QuartzCore
                    CoreMedia
                    WebKit
                )
        
        FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )
        list ( APPEND LIST_SHARED_LIBRARIES  ${${NAME}_LIBRARY} )
    endforeach()

    list ( APPEND LIST_SHARED_LIBRARIES ${LIST_LIBRARIES} )                           

    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.a" )
    list ( APPEND LIST_SHARED_LIBRARIES ${LIB_LIST}  )   

    set_source_files_properties( ${CMAKE_CURRENT_LIST_DIR}/Platform/ThreadPosix.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++" )  

elseif( IOS )  
  
    set ( LIBRARY_TYPE STATIC )

    add_definitions( -DDDARWIN_NO_CARBON ) 
    
    set ( CMAKE_C_FLAGS    "-mno-thumb"  )
    set ( CMAKE_CXX_FLAGS  "-mno-thumb -fvisibility=hidden" )
    set ( LIST_SHARED_LIBRARIES 
                    libz.dylib 
                    libbz2.dylib
                )

    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    foreach( NAME  CoreGraphics 
                   Foundation 
                   UIKit 
                   OpenGLES
                   OpenAL
                   CoreAudio
                   AVFoundation
                   AudioToolbox
                   QuartzCore
                   GameKit
                   MessageUI
                   AdSupport
                   GameController
                   Security
                   AssetsLibrary
                   SystemConfiguration
                   MediaPlayer
                )
        
        FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )
        list ( APPEND LIST_SHARED_LIBRARIES  ${${NAME}_LIBRARY} )
    endforeach()

    file ( GLOB RESOURCE "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.a" )
    list ( APPEND LIST_SHARED_LIBRARIES ${RESOURCE}  )   

    set_source_files_properties( ${CMAKE_CURRENT_LIST_DIR}/Platform/ThreadPosix.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++" )  


elseif( WIN32 ) 
    set ( LIBRARY_TYPE STATIC ) 

    if( NOT DISABLE_MT )
        set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP" ) 
        set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP" )
    endif()
    
    if ( MSVC )
        file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/D/*" )
        list ( APPEND LIST_SHARED_LIBRARIES_DEBUG ${LIB_LIST}  )

        file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.lib" )
        list ( APPEND LIST_SHARED_LIBRARIES_RELEASE ${LIB_LIST}  )

        set  ( LIST_SHARED_LIBRARIES "opengl32.lib" "ws2_32.lib" "winmm.lib" "wldap32.lib" "iphlpapi.lib"  )
  
    endif()

endif()


# Define source 
define_source_folders  ()
generate_source_groups_project ()

include_directories    (.)
include_directories    ( ${DAVA_INCLUDE_DIR} )
include_directories    ( ${ADD_ENGINE_INCLUDES_DIR} ) 
include_directories    ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} ) 


add_definitions        ( ${FRAMEWORK_DEFINITIONS} )
add_library            ( ${PROJECT_NAME} ${LIBRARY_TYPE} ${PROJECT_SOURCE_FILES} ${RESOURCE} )


target_link_libraries  ( ${PROJECT_NAME} ${LIST_SHARED_LIBRARIES} )  

foreach ( FILE ${LIST_SHARED_LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIST_SHARED_LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()

install_libraries( ${PROJECT_NAME} )

