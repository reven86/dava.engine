cmake_minimum_required( VERSION 3.0 )

project( DavaFramework )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../CMake/Modules/" ) 

include( CMake-common )

# STEAM_SDK_FOUND inits from Sources/CMake/Modules/FindSteam.cmake
# it inits only if STEAM definition passed as command line parameter
if ( STEAM_SDK_FOUND )
    include_directories( ${STEAM_SDK_HEADERS} )
    add_definitions ( -D__DAVAENGINE_STEAM__ )
endif ()

set ( FRAMEWORK_DEFINITIONS 
-D_UNICODE 
-DUNICODE 
-D_CRT_SECURE_NO_DEPRECATE 
-D_CRT_NON_CONFORMING_SWPRINTFS
-DCURL_STATICLIB
)

if( DAVA_DISABLE_AUTOTESTS  )
    list( APPEND FRAMEWORK_DEFINITIONS -DDISABLE_AUTOTESTS )
endif()

if( NOT DISABLE_SOUNDS )
    list( APPEND FRAMEWORK_DEFINITIONS -DDAVA_FMOD )

endif()

if( DAVA_USE_RENDERSTATS )
    list( APPEND FRAMEWORK_DEFINITIONS -D__DAVAENGINE_RENDERSTATS__ )
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
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent.so"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex.so"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_android.a"
         "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp_android.a"
         )

    list ( APPEND LIST_LIBRARIES
         "-lEGL"
         "-lGLESv1_CM"
         "-llog"
         "-landroid"
         "-lGLESv2"
         "-lstdc++" 
         "-latomic"
         )

    list ( APPEND LIST_SHARED_LIBRARIES ${LIST_LIBRARIES} )

    set ( FRAMEWORK_DEFINITIONS  ${FRAMEWORK_DEFINITIONS} -D__ANDROID__ )
    set ( LIBRARY_OUTPUT_PATH    ${THIRD_PARTY_LIBRARIES_PATH} )

    set ( IGNORED_RHI_FOLDER "Metal" "DX9" "DX11" )

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
                    SystemConfiguration
                    IOKit
                )
        
        FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )
        list ( APPEND LIST_ADDED_SHARED_LIBRARIES  ${${NAME}_LIBRARY} )
    endforeach()

    list ( APPEND LIST_SHARED_LIBRARIES ${LIST_LIBRARIES} )                           

    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.a" )
    list ( APPEND LIST_SHARED_LIBRARIES ${LIB_LIST}  )   
    
    set ( IGNORED_RHI_FOLDER "Metal" "DX9" "DX11" )

elseif( IOS )  

    if ( NOT NOT_TARGET_EXECUTABLE )
        find_package( ZLIB )
    endif()

    add_definitions( -DDDARWIN_NO_CARBON ) 
    
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

    set ( IGNORED_RHI_FOLDER "DX9" "DX11" )

elseif ( WINDOWS_UAP )
    add_static_libs_win_uap ( "${DAVA_WIN_UAP_LIBRARIES_PATH_COMMON}" LIST_SHARED_LIBRARIES )

    # list each library on its own line to ease library addition
    list ( APPEND LIST_SHARED_LIBRARIES
            "d2d1.lib"
            "d3d11.lib"
            "d3dcompiler.lib"
            "dxgi.lib"
            "dxguid.lib"
            "dwrite.lib"
            "iphlpapi.lib"
    )
    set ( IGNORED_RHI_FOLDER "DX9" "GLES2" "Metal" )

elseif( WIN32 ) 
    include( FindWindowsSDK )
    include_directories   ( ${WINDOWSSDK_LATEST_DIR}/include/shared ) 
    include_directories   ( $ENV{DXSDK_DIR}/include ) 

#    link_directories       ( $ENV{DXSDK_DIR}/lib/x86 )
    link_directories       ( ${WINDOWSSDK_LATEST_DIR}/lib/x86 )
    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/*" )
    list ( APPEND LIST_SHARED_LIBRARIES_DEBUG ${LIB_LIST}  )

    file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/*.lib" )
    list ( APPEND LIST_SHARED_LIBRARIES_RELEASE ${LIB_LIST}  )

    # list each library on its own line to ease library addition
    list ( APPEND LIST_SHARED_LIBRARIES
            "Wininet.lib"
            "opengl32.lib"
            "ws2_32.lib"
            "winmm.lib"
            "wldap32.lib"
            "iphlpapi.lib"
            "Gdi32.lib"
            "glu32.lib"
            "psapi.lib"
            "dbghelp.lib"
            "userenv.lib"
            "delayimp.lib"
            "dxgi.lib"
	)

    list ( APPEND LIST_SHARED_LIBRARIES_RELEASE "msvcrt.lib"  )
    list ( APPEND LIST_SHARED_LIBRARIES_DEBUG   "msvcrtd.lib" )

    if( X64_MODE )
	    list ( APPEND LIST_SHARED_LIBRARIES
            "$ENV{DXSDK_DIR}/lib/x64/d3dx9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3d9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3d11.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/d3dcompiler.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x64/dxguid.lib"
    	)
	else ()
	    list ( APPEND LIST_SHARED_LIBRARIES
            "$ENV{DXSDK_DIR}/lib/x86/d3dx9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3d9.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3d11.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/d3dcompiler.lib"
            "${WINDOWSSDK_LATEST_DIR}/lib/winv6.3/um/x86/dxguid.lib"
    	)
	endif ()

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
generated_unity_sources     ( PROJECT_SOURCE_FILES IGNORE_LIST        
                                                        "Internal/Autotesting"
                                                   IGNORE_LIST_APPLE  
                                                        "DataStorage/Apple/ICloudKeyValue.cpp"
                                                        "TemplateMacOS/CoreMacOSPlatformBase.cpp"
                                                        "Notification/LocalNotificationIOS.cpp"
                                                        "Notification/LocalNotificationMac.cpp"
                                                        "Concurrency/ThreadPosix.cpp"
                                                        "Render/Texture.cpp" 
                                                        "Render/RHI/GLES2"
                                                        "TemplateMacOS/MainWindowController.mm"
                                                        "TemplateMacOS/MovieViewControlMacOS.mm"
                                                        "Logger/TeamcityOutput.cpp"
                                                    IGNORE_LIST_IOS
                                                        "DataStorage/Apple/ICloudKeyValue.cpp"
                                                        "TemplateiOS/BackgroundView.mm"
                                                        "TemplateiOS/HelperAppDelegate.mm"
                                                        "TemplateiOS/DPIHelperiOS.mm"
                                                        "TemplateiOS"
                                                        "UI/UITextFieldiPhone.mm"
                                                   )

include_directories      (.)
include_directories      ( ${DAVA_INCLUDE_DIR} )
include_directories      ( ${ADD_ENGINE_INCLUDES_DIR} ) 
include_directories      ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} ) 
include_directories      ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} ) 
include_directories      ( ${DAVA_THIRD_PARTY_ROOT_PATH}/include/ffmpeg )

set                      ( DAVA_INCLUDE_DIR ${DAVA_INCLUDE_DIR} ${ADD_ENGINE_INCLUDES_DIR} PARENT_SCOPE )

add_definitions          ( ${FRAMEWORK_DEFINITIONS} )
add_library              ( ${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES} ${RESOURCE} )

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

if ( WINDOWS_UAP )
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${WINDOWS_UAP_MOBILE_EXT_SDK_VERSION} )
endif()
