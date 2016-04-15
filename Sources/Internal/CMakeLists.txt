set( NAME_MODULE DavaFramework )


set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( INCLUDES_PRIVATE               ${DAVA_ENGINE_DIR} 
                                    ${DAVA_THIRD_PARTY_INCLUDES_PATH}  )

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

set( DYNAMIC_LIBRARIES_ANDROID      ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libiconv_android.so )
                               
set( SOURCE_FOLDERS                 "." )

set( ERASE_FOLDERS_IOS              "DX9" "DX11" )
set( ERASE_FOLDERS_MACOS            "Metal" "DX9" "DX11" )
set( ERASE_FOLDERS_ANDROID          "Metal" "DX9" "DX11" )
set( ERASE_FOLDERS_WIN32            "Metal" )


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
    list( APPEND DEFINITIONS_PRIVATE -DDISABLE_AUTOTESTS )
endif()


setup_main_module()
