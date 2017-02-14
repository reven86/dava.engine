set( NAME_MODULE Sound )

set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( HPP_FILES                       Sources/*.h Sources/*.hpp )
set( CPP_FILES                       Sources/Private/*.cpp )
set( CPP_FILES_IOS                   Sources/Private/iOS/*.mm )

set( LIBRARIES_FMOD                  ${CMAKE_CURRENT_LIST_DIR}/Libs/lib )

set( INCLUDES                        ${CMAKE_CURRENT_LIST_DIR}/Libs/include 
                                     ${CMAKE_CURRENT_LIST_DIR}/Sources 
                                     ${DAVA_ENGINE_DIR} 
                                     ${DAVA_THIRD_PARTY_ROOT_PATH}/include )

set( STATIC_LIBRARIES_IOS            ${LIBRARIES_FMOD}/ios/libfmodevent_iphoneos.a 
                                     ${LIBRARIES_FMOD}/ios/libfmodex_iphoneos.a )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${LIBRARIES_FMOD}/win/x86/Release/fmod_event.lib 
                                     ${LIBRARIES_FMOD}/win/x86/Release/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${LIBRARIES_FMOD}/win/x86/Debug/fmod_event.lib 
                                     ${LIBRARIES_FMOD}/win/x86/Debug/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN64_RELEASE  ${LIBRARIES_FMOD}/win/x64/Release/fmod_event.lib 
                                     ${LIBRARIES_FMOD}/win/x64/Release/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${LIBRARIES_FMOD}/win/x64/Debug/fmod_event.lib 
                                     ${LIBRARIES_FMOD}/win/x64/Debug/fmodex_vc.lib )


set( STATIC_LIBRARIES_WINUAP_RELEASE ${LIBRARIES_FMOD}/win10/$(Platform)/Release/fmod_eventUWP.lib 
                                     ${LIBRARIES_FMOD}/win10/$(Platform)/Release/fmodexUWP_vc.lib )

set( STATIC_LIBRARIES_WINUAP_DEBUG   ${LIBRARIES_FMOD}/win10/$(Platform)/Debug/fmod_eventUWP.lib 
                                     ${LIBRARIES_FMOD}/win10/$(Platform)/Debug/fmodexUWP_vc.lib )


set( DYNAMIC_LIBRARIES_ANDROID       ${LIBRARIES_FMOD}/android/${ANDROID_NDK_ABI_NAME}/libfmodevent.so 
                                     ${LIBRARIES_FMOD}/android/${ANDROID_NDK_ABI_NAME}/libfmodex.so )

set( DYNAMIC_LIBRARIES_WIN32         ${LIBRARIES_FMOD}/win/x86/Release/fmodex.dll 
                                     ${LIBRARIES_FMOD}/win/x86/Release/fmod_event.dll )

set( DYNAMIC_LIBRARIES_WIN64         ${LIBRARIES_FMOD}/win/x64/Release/fmodex64.dll 
                                     ${LIBRARIES_FMOD}/win/x64/Release/fmod_event64.dll )

set( DYNAMIC_LIBRARIES_MACOS         ${LIBRARIES_FMOD}/macos/libfmodevent.dylib
                                     ${LIBRARIES_FMOD}/macos/libfmodex.dylib )


set( DYNAMIC_LIBRARIES_WINUAP        ${LIBRARIES_FMOD}/win10/$(Platform)/Release/fmod_eventUWParm.dll 
                                     ${LIBRARIES_FMOD}/win10/$(Platform)/Release/fmodexUWParm.dll )

set( JAR_FOLDERS_ANDROID             ${LIBRARIES_FMOD}/android/jar )

set(EXCLUDE_FROM_ALL true )

setup_main_module()









