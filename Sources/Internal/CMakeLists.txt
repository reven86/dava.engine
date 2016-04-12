set( NAME_MODULE DavaFramework )


set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( INCLUDES_PRIVATE           ${DAVA_ENGINE_DIR} 
                                ${DAVA_THIRD_PARTY_INCLUDES_PATH}  )

set( DEFINITIONS_PRIVATE        -D_UNICODE 
                                -DUNICODE 
                                -D_CRT_SECURE_NO_DEPRECATE 
                                -D_CRT_NON_CONFORMING_SWPRINTFS
                                -DCURL_STATICLIB  )

set( DEFINITIONS_PRIVATE_MACOS  -D__DAVAENGINE_PTHREAD__ )
   

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



if( DAVA_DISABLE_AUTOTESTS  )
    list( APPEND DEFINITIONS_PRIVATE -DDISABLE_AUTOTESTS )
endif()

if( MACOS )
    find_package( ZLIB )
    list (APPEND INCLUDES_PRIVATE  ${ZLIB_INCLUDE_DIR} )
    list (APPEND DYNAMIC_LIBRARIES_MACOS ${ZLIB_LIBRARY}     )
endif()

setup_main_module()
