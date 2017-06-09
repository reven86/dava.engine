set( NAME_MODULE Sound )

set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( HPP_FILES                       Sources/*.h Sources/Private/*.h )
set( CPP_FILES                       Sources/Private/*.cpp )
set( CPP_FILES_IOS                   Sources/Private/iOS/*.mm )

set( LIBRARIES_FMOD                  ${CMAKE_CURRENT_LIST_DIR}/Libs )

set( INCLUDES                        ${LIBRARIES_FMOD}/Include 
                                     ${CMAKE_CURRENT_LIST_DIR}/Sources 
                                     ${DAVA_ENGINE_DIR} 
                                     ${DAVA_THIRD_PARTY_ROOT_PATH}/include )

set( STATIC_LIBRARIES_IOS            ${LIBRARIES_FMOD}/Ios/libfmodevent_iphoneos.a 
                                     ${LIBRARIES_FMOD}/Ios/libfmodex_iphoneos.a )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${LIBRARIES_FMOD}/Win/x86/Release/fmod_event.lib 
                                     ${LIBRARIES_FMOD}/Win/x86/Release/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${LIBRARIES_FMOD}/Win/x86/Debug/fmod_event.lib 
                                     ${LIBRARIES_FMOD}/Win/x86/Debug/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN64_RELEASE  ${LIBRARIES_FMOD}/Win/x64/Release/fmod_event64.lib 
                                     ${LIBRARIES_FMOD}/Win/x64/Release/fmodex64_vc.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${LIBRARIES_FMOD}/Win/x64/Debug/fmod_event64.lib 
                                     ${LIBRARIES_FMOD}/Win/x64/Debug/fmodex64_vc.lib )


set( STATIC_LIBRARIES_WINUAP_RELEASE ${LIBRARIES_FMOD}/Win10/$(Platform)/Release/fmod_eventUWP.lib 
                                     ${LIBRARIES_FMOD}/Win10/$(Platform)/Release/fmodexUWP_vc.lib )

set( STATIC_LIBRARIES_WINUAP_DEBUG   ${LIBRARIES_FMOD}/Win10/$(Platform)/Debug/fmod_eventUWPD.lib 
                                     ${LIBRARIES_FMOD}/Win10/$(Platform)/Debug/fmodexUWPD_vc.lib )


set( DYNAMIC_LIBRARIES_ANDROID       ${LIBRARIES_FMOD}/Android/${ANDROID_NDK_ABI_NAME}/libfmodevent.so 
                                     ${LIBRARIES_FMOD}/Android/${ANDROID_NDK_ABI_NAME}/libfmodex.so )

set( DYNAMIC_LIBRARIES_WIN32_RELEASE ${LIBRARIES_FMOD}/Win/x86/Release/fmodex.dll 
                                     ${LIBRARIES_FMOD}/Win/x86/Release/fmod_event.dll )

set( DYNAMIC_LIBRARIES_WIN64_RELEASE ${LIBRARIES_FMOD}/Win/x64/Release/fmodex64.dll 
                                     ${LIBRARIES_FMOD}/Win/x64/Release/fmod_event64.dll )


set( DYNAMIC_LIBRARIES_WIN32_DEBUG   ${LIBRARIES_FMOD}/Win/x86/Debug/fmodex.dll 
                                     ${LIBRARIES_FMOD}/Win/x86/Debug/fmod_event.dll )

set( DYNAMIC_LIBRARIES_WIN64_DEBUG   ${LIBRARIES_FMOD}/Win/x64/Debug/fmodex64.dll 
                                     ${LIBRARIES_FMOD}/Win/x64/Debug/fmod_event64.dll )


set( DYNAMIC_LIBRARIES_MACOS         ${LIBRARIES_FMOD}/Macos/libfmodevent.dylib
                                     ${LIBRARIES_FMOD}/Macos/libfmodex.dylib )

set( DYNAMIC_LIBRARIES_WINUAP_RELEASE ${LIBRARIES_FMOD}/win10/Win32/RELEASE/fmod_eventUWP.dll 
                                      ${LIBRARIES_FMOD}/win10/Win32/RELEASE/fmodexUWP.dll

                                      ${LIBRARIES_FMOD}/win10/arm/RELEASE/fmod_eventUWParm.dll
                                      ${LIBRARIES_FMOD}/win10/arm/RELEASE/fmodexUWParm.dll

                                      ${LIBRARIES_FMOD}/win10/x64/RELEASE/fmod_eventUWP64.dll 
                                      ${LIBRARIES_FMOD}/win10/x64/RELEASE/fmodexUWP64.dll )

set( DYNAMIC_LIBRARIES_WINUAP_DEBUG   ${LIBRARIES_FMOD}/win10/Win32/DEBUG/fmod_eventUWPD.dll 
                                      ${LIBRARIES_FMOD}/win10/Win32/DEBUG/fmodexUWPD.dll

                                      ${LIBRARIES_FMOD}/win10/arm/DEBUG/fmod_eventUWPDarm.dll
                                      ${LIBRARIES_FMOD}/win10/arm/DEBUG/fmodexUWPDarm.dll

                                      ${LIBRARIES_FMOD}/win10/x64/DEBUG/fmod_eventUWPD64.dll 
                                      ${LIBRARIES_FMOD}/win10/x64/DEBUG/fmodexUWPD64.dll )

set( JAR_FOLDERS_ANDROID             ${LIBRARIES_FMOD}/Android/jar )

setup_main_module()









