
set( NAME_MODULE Sound )

set( MODULE_TYPE INLINE )#INLINE STATIC DYNAMIC 

set( HPP_FILES                       *.h *.hpp            )
set( CPP_FILES                       *.cpp                )

set( HPP_FILES_IOS                   iOS/*.h              )
set( CPP_FILES_IOS                   iOS/*.mm  iOS/*.cpp  )

set( ERASE_FILES_IOS                 SoundSystemEmpty.cpp )

set( DEFINITIONS                     -DDAVA_FMOD          )

set( INCLUDES                        ${DAVA_THIRD_PARTY_ROOT_PATH}/fmod/include ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_ROOT_PATH}/include )

set( STATIC_LIBRARIES_IOS            ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent_iphoneos.a 
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex_iphoneos.a )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/fmod_event.lib 
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/fmod_event.lib 
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN64_RELEASE  ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/fmod_event.lib 
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/fmodex_vc.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/fmod_event.lib 
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/fmodex_vc.lib )

set( DYNAMIC_LIBRARIES_ANDROID       ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent.so 
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex.so )

set( DYNAMIC_LIBRARIES_WIN32         ${DAVA_TOOLS_BIN_DIR}/fmodex.dll 
                                     ${DAVA_TOOLS_BIN_DIR}/fmod_event.dll )

set( DYNAMIC_LIBRARIES_WIN64         ${DAVA_TOOLS_BIN_DIR}/fmodex64.dll 
                                     ${DAVA_TOOLS_BIN_DIR}/fmod_event64.dll )

set( DYNAMIC_LIBRARIES_MACOS         ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent.dylib
                                     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex.dylib )

setup_main_module()









