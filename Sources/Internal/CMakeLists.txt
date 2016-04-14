set( NAME_MODULE DavaFramework )


set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( INCLUDES_PRIVATE           ${DAVA_ENGINE_DIR} 
                                ${DAVA_THIRD_PARTY_INCLUDES_PATH}  )

   
set( FIND_SYSTEM_LIBRARY_IOS    CoreGraphics 
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
                                ZLIB 
                                )

set( FIND_SYSTEM_LIBRARY_MACOS  Cocoa
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
                                ZLIB
                                )

set( STATIC_LIBRARIES_IOS       ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto_ios_release.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl_ios_release.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_ios.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_ios.a
                                )

set( STATIC_LIBRARIES_MACOS     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libFColladaS.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libPVRTexLib.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto_macos_release.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libdxt_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl_macos_release.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbis_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbisfile_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a 
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_macos.a
                                ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_macos.a )

                               
set( SOURCE_FOLDERS             "." )

set( ERASE_FOLDERS_IOS          "DX9" "DX11" )
set( ERASE_FOLDERS_MACOS        "Metal" "DX9" "DX11" )


set( UNITY_IGNORE_LIST          "Internal/Autotesting" )

set( UNITY_IGNORE_LIST_APPLE    "DataStorage/Apple/ICloudKeyValue.cpp"
                                "TemplateMacOS/CoreMacOSPlatformBase.cpp"
                                "Notification/LocalNotificationIOS.cpp"
                                "Notification/LocalNotificationMac.cpp"
                                "Concurrency/ThreadPosix.cpp"
                                "Render/Texture.cpp" 
                                "Render/RHI/GLES2"
                                "TemplateMacOS/MainWindowController.mm"
                                "TemplateMacOS/MovieViewControlMacOS.mm"
                                "Logger/TeamcityOutput.cpp" )

set( UNITY_IGNORE_LIST_IOS      "DataStorage/Apple/ICloudKeyValue.cpp"
                                "TemplateiOS/BackgroundView.mm"
                                "TemplateiOS/HelperAppDelegate.mm"
                                "TemplateiOS/DPIHelperiOS.mm"
                                "TemplateiOS"
                                "UI/UITextFieldiPhone.mm" )

set( DEFINITIONS_PRIVATE        -D_UNICODE 
                                -DUNICODE 
                                -D_CRT_SECURE_NO_DEPRECATE 
                                -D_CRT_NON_CONFORMING_SWPRINTFS
                                -DCURL_STATICLIB  )

set( DEFINITIONS_PRIVATE_IOS    -DDDARWIN_NO_CARBON )
set( DEFINITIONS_PRIVATE_MACOS  -D__DAVAENGINE_PTHREAD__ )


if( DAVA_DISABLE_AUTOTESTS  )
    list( APPEND DEFINITIONS_PRIVATE -DDISABLE_AUTOTESTS )
endif()


setup_main_module()
