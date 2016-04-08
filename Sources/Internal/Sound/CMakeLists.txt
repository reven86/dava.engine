
set( NAME_MODULE Sound )

set( MODULE_TYPE INLINE )#INLINE STATIC DINAMIC 

set( HPP_FILES                   *.h *.hpp            )
set( CPP_FILES                   *.cpp                )

set( HPP_FILES_IOS               iOS/*.h              )
set( CPP_FILES_IOS               iOS/*.mm  iOS/*.cpp  )

set( ERASE_FILES_IOS             SoundSystemEmpty.cpp )

set( DEFINITIONS                 -DDAVA_FMOD          )

set( ADD_INCLUDES_DIR            ${DAVA_THIRD_PARTY_ROOT_PATH}/fmod/include ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_ROOT_PATH}/include )

set( STATIC_LIBRARIES_IOS        ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent_iphoneos.a 
	                             ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex_iphoneos.a )

set( DINAMIC_LIBRARIES_ANDROID   ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent.so 
	                             ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex.so )

set( DINAMIC_LIBRARIES_WINDOWS   ${DAVA_TOOLS_BIN_DIR}/fmodex.dll 
	                             ${DAVA_TOOLS_BIN_DIR}/fmod_event.dll )

set( DINAMIC_LIBRARIES_MACOS     ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodevent.dylib
	                             ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfmodex.dylib )

setup_main_module()









