cmake_minimum_required( VERSION 3.0 )

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
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )

set ( IGNORED_RHI_FOLDER "" )

if( ANDROID )
    find_package( ZLIB )

    set ( LIST_LIBRARIES
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libzip_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfribidi_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libiconv_android.so"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent.so"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex.so"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libTextureConverter_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libbox2d.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libiconv_android.so"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_android.a"
         )

    list ( APPEND LIST_LIBRARIES
         "-lEGL"
         "-lGLESv1_CM"
         "-llog"
         "-landroid"
         "-lGLESv2"
         "-lstdc++" 
         )

    list ( APPEND LIST_SHARED_LIBRARIES ${LIST_LIBRARIES} )

    set ( FRAMEWORK_DEFINITIONS  ${FRAMEWORK_DEFINITIONS} -D__ANDROID__ )
    set ( LIBRARY_OUTPUT_PATH    ${THIRD_PARTY_LIBRARIES_PATH} )

    set ( IGNORED_RHI_FOLDER "Metal" "DX9" )

elseif( MACOS )
    find_package( ZLIB )
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
        list ( APPEND LIST_ADDED_SHARED_LIBRARIES  ${${NAME}_LIBRARY} )
    endforeach()

    list ( APPEND LIST_SHARED_LIBRARIES ${LIST_LIBRARIES} )                           

    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.a" )
    list ( APPEND LIST_SHARED_LIBRARIES ${LIB_LIST}  )   
    
    set ( IGNORED_RHI_FOLDER "Metal" "DX9" )

elseif( IOS )  
    find_package( ZLIB )

    add_definitions( -DDARWIN_NO_CARBON ) 
    
    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    foreach( NAME  CoreGraphics 
                   Foundation 
                   UIKit 
                   OpenGLES
                   Metal
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
        list ( APPEND LIST_ADDED_SHARED_LIBRARIES  ${${NAME}_LIBRARY} )
    endforeach()

    file ( GLOB RESOURCE "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.a" )
    list ( APPEND LIST_SHARED_LIBRARIES ${RESOURCE}  )    

    set ( IGNORED_RHI_FOLDER "DX9" )

elseif ( WINDOWS_UAP )

    set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/angle-ms-master/include" )
    set ( DAVA_WIN_STORE_LIBRARIES_PATH "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win10" ) 

    #file ( GLOB SHARED_LIB_LIST "${DAVA_WIN_STORE_LIBRARIES_PATH}/*.lib" )
    #list ( APPEND LIST_SHARED_LIBRARIES ${SHARED_LIB_LIST}  )
    #list ( APPEND LIST_SHARED_LIBRARIES_RELEASE  "" )
    #list ( APPEND LIST_SHARED_LIBRARIES_DEBUG   "" )

    list ( APPEND LIST_SHARED_LIBRARIES    "Wininet.lib" "ws2_32.lib" "winmm.lib" "wldap32.lib" "iphlpapi.lib" "Gdi32.lib" "dbghelp.lib" "d2d1.lib" "d3d11.lib" "d3dcompiler.lib" "dxgi.lib" "windowscodecs.lib" "dwrite.lib" )
    
    #choose target architecture libraries
    if( "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "ARM" )
        set ( LIBS_ARCH_SELECTOR "arm" )
    elseif ( "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64" )
        set ( LIBS_ARCH_SELECTOR "x64" )
    else ()
        set ( LIBS_ARCH_SELECTOR "x86" )
    endif ()

    file ( GLOB DEBUG_LIB_LIST "${DAVA_WIN_STORE_LIBRARIES_PATH}/${LIBS_ARCH_SELECTOR}/Debug/*.lib" )
    list ( APPEND LIST_SHARED_LIBRARIES_DEBUG ${DEBUG_LIB_LIST}  )

    file ( GLOB RELEASE_LIB_LIST "${DAVA_WIN_STORE_LIBRARIES_PATH}/${LIBS_ARCH_SELECTOR}/Release/*.lib" )
    list ( APPEND LIST_SHARED_LIBRARIES_RELEASE ${RELEASE_LIB_LIST}  )

elseif( MSVC ) 

    link_directories       ( $ENV{DXSDK_DIR}/lib/x86 )

    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/D/*" )
    list ( APPEND LIST_SHARED_LIBRARIES_DEBUG ${LIB_LIST}  )

    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.lib" )
    list ( APPEND LIST_SHARED_LIBRARIES_RELEASE ${LIB_LIST}  )

    list ( APPEND LIST_SHARED_LIBRARIES         "Wininet.lib" "opengl32.lib" "ws2_32.lib" "winmm.lib" "wldap32.lib" "iphlpapi.lib" "Gdi32.lib" "glu32.lib" "psapi.lib" "dbghelp.lib" "$ENV{DXSDK_DIR}/lib/x86/d3d9.lib" "$ENV{DXSDK_DIR}/lib/x86/d3dx9.lib" "$ENV{DXSDK_DIR}/lib/x86/d3d11.lib" "$ENV{DXSDK_DIR}/lib/x86/dxguid.lib" )

    list ( APPEND LIST_SHARED_LIBRARIES_RELEASE "msvcrt.lib"  )
    list ( APPEND LIST_SHARED_LIBRARIES_DEBUG   "msvcrtd.lib" )

    set ( IGNORED_RHI_FOLDER "Metal" )

endif()

if( ZLIB_FOUND )
    list (APPEND DAVA_THIRD_PARTY_INCLUDES_PATH ${ZLIB_INCLUDE_DIR} )
    list (APPEND LIST_SHARED_LIBRARIES          ${ZLIB_LIBRARY}     )
endif()

# Define source 

define_source_folders  ( SRC_ROOT "." GLOB_ERASE_FOLDERS "${IGNORED_RHI_FOLDER}" )

generate_source_groups_project ()

set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )

include_directories    (.)
include_directories    ( ${DAVA_INCLUDE_DIR} )
include_directories    ( ${ADD_ENGINE_INCLUDES_DIR} ) 
include_directories    ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} ) 
include_directories    ( $ENV{DXSDK_DIR}/include ) 


set                    ( DAVA_INCLUDE_DIR ${DAVA_INCLUDE_DIR} ${ADD_ENGINE_INCLUDES_DIR} PARENT_SCOPE )

add_definitions        ( ${FRAMEWORK_DEFINITIONS} )
add_library            ( ${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES} ${RESOURCE} )

if( LINK_THIRD_PARTY )                 
   MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} ALL     "${LIST_SHARED_LIBRARIES}" )
   MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} DEBUG   "${LIST_SHARED_LIBRARIES_DEBUG}" )
   MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} RELEASE "${LIST_SHARED_LIBRARIES_RELEASE}" )
endif()

target_link_libraries  ( ${PROJECT_NAME} ${LIST_SHARED_LIBRARIES} ${LIST_ADDED_SHARED_LIBRARIES} )  

foreach ( FILE ${LIST_SHARED_LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIST_SHARED_LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()

install_libraries( ${PROJECT_NAME} )


