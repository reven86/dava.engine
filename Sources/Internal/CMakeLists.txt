set( NAME_MODULE DavaFramework )

set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( INCLUDES_PRIVATE               ${DAVA_ENGINE_DIR} 
                                    ${DAVA_THIRD_PARTY_INCLUDES_PATH}  )

set( INCLUDES_WIN32                 ${WINDOWSSDK_LATEST_DIR}/include/shared        
                                    $ENV{DXSDK_DIR}/include )
  
set( FIND_SYSTEM_LIBRARY_IOS        CoreGraphics 
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
                                    ZLIB )

set( FIND_SYSTEM_LIBRARY_MACOS      Cocoa
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
                                    ZLIB )


set( FIND_SYSTEM_LIBRARY_ANDROID    ZLIB ) 

set( STATIC_LIBRARIES_IOS           ${DAVA_STATIC_LIBRARIES_IOS} )
set( STATIC_LIBRARIES_MACOS         ${DAVA_STATIC_LIBRARIES_MACOS} )
set( STATIC_LIBRARIES_ANDROID       ${DAVA_STATIC_LIBRARIES_ANDROID} )  
set( STATIC_LIBRARIES_WIN32         ${DAVA_STATIC_LIBRARIES_WIN32} )
set( STATIC_LIBRARIES_WIN32_RELEASE ${DAVA_STATIC_LIBRARIES_WIN32_RELEASE} )
set( STATIC_LIBRARIES_WIN32_DEBUG   ${DAVA_STATIC_LIBRARIES_WIN32_DEBUG} )

set( STATIC_LIBRARIES_WINUAP        ${DAVA_STATIC_LIBRARIES_WINUAP} )
set( STATIC_LIBRARIES_WINUAP_RELEASE ${DAVA_STATIC_LIBRARIES_WINUAP_RELEASE} )
set( STATIC_LIBRARIES_WINUAP_DEBUG  ${DAVA_STATIC_LIBRARIES_WINUAP_DEBUG} )


set( DYNAMIC_LIBRARIES_ANDROID      ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libiconv_android.so )
                               
set( SOURCE_FOLDERS                 "." )

set( CPP_FILES                      ${DAVA_EXTERNAL_DIR}/lz4/*.c )
set( HPP_FILES                      ${DAVA_EXTERNAL_DIR}/lz4/*.h )
                   
set( ERASE_FOLDERS_IOS              "DX9" "DX11" )
set( ERASE_FOLDERS_MACOS            "Metal" "DX9" "DX11" )
set( ERASE_FOLDERS_ANDROID          "Metal" "DX9" "DX11" )
set( ERASE_FOLDERS_WIN32            "Metal" )
set( ERASE_FOLDERS_WINUAP           "DX9" "GLES2" "Metal" )


set( UNITY_IGNORE_LIST              "Internal/Autotesting" )

set( UNITY_IGNORE_LIST_APPLE        "DataStorage/Apple/ICloudKeyValue.cpp"
                                    "TemplateMacOS/CoreMacOSPlatformBase.cpp"
                                    "Notification/LocalNotificationIOS.cpp"
                                    "Notification/LocalNotificationMac.cpp"
                                    "Concurrency/ThreadPosix.cpp"
                                    "Render/Texture.cpp" 
                                    "Render/RHI/GLES2"
                                    "TemplateMacOS/MainWindowController.mm"
                                    "TemplateMacOS/MovieViewControlMacOS.mm"
                                    "Logger/TeamcityOutput.cpp" )

set( UNITY_IGNORE_LIST_IOS          "DataStorage/Apple/ICloudKeyValue.cpp"
                                    "TemplateiOS/BackgroundView.mm"
                                    "TemplateiOS/HelperAppDelegate.mm"
                                    "TemplateiOS/DPIHelperiOS.mm"
                                    "TemplateiOS"
                                    "UI/UITextFieldiPhone.mm" )

set( DEFINITIONS_PRIVATE            -D_UNICODE 
                                    -DUNICODE 
                                    -D_CRT_SECURE_NO_DEPRECATE 
                                    -D_CRT_NON_CONFORMING_SWPRINTFS
                                    -DCURL_STATICLIB  )

set( DEFINITIONS_PRIVATE_IOS        -DDDARWIN_NO_CARBON )
set( DEFINITIONS_PRIVATE_MACOS      -D__DAVAENGINE_PTHREAD__ )
set( DEFINITIONS_PRIVATE_ANDROID    -D__ANDROID__ )


if( DAVA_DISABLE_AUTOTESTS  )
    list( APPEND DEFINITIONS -DDISABLE_AUTOTESTS )
endif()

if ( STEAM_SDK_FOUND )#tmp
    list( APPEND INCLUDES ${STEAM_SDK_HEADERS} )
    list( APPEND DEFINITIONS -D__DAVAENGINE_STEAM__ )
endif ()

if( DAVA_USE_RENDERSTATS )
    list( APPEND DEFINITIONS -D__DAVAENGINE_RENDERSTATS__ )
endif()

setup_main_module()
