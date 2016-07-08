if ( NOT STEAM )
    return ()
endif ()

if (STEAM_SDK_FOUND)
    return ()
endif ()
 
set (STEAM_SDK_FOUND true)

set (STEAM_SDK "${DAVA_THIRD_PARTY_ROOT_PATH}/steam_sdk")
set (STEAM_SDK_HEADERS ${STEAM_SDK}/public)

if (WIN32)
    set (STEAM_SDK_DYNAMIC_LIBRARIES_PATH ${STEAM_SDK}/redistributable_bin)
    set (STEAM_SDK_DYNAMIC_LIBRARIES ${STEAM_SDK}/redistributable_bin/steam_api.dll)
    set (STEAM_SDK_STATIC_LIBRARIES ${STEAM_SDK}/redistributable_bin/steam_api.lib
                                    ${STEAM_SDK}/public/steam/lib/win32/sdkencryptedappticket.lib)
elseif (MACOS)
    set (STEAM_SDK_DYNAMIC_LIBRARIES ${STEAM_SDK}/redistributable_bin/osx32/libsteam_api.dylib
                                     ${STEAM_SDK}/public/steam/lib/osx32/libsdkencryptedappticket.dylib)
    set (STEAM_SDK_STATIC_LIBRARIES )
endif ()

message ("STEAM_SDK_DYNAMIC_LIBRARIES_PATH - ${STEAM_SDK_DYNAMIC_LIBRARIES_PATH}")
message ("STEAM_SDK_DYNAMIC_LIBRARIES - ${STEAM_SDK_DYNAMIC_LIBRARIES}")
message ("STEAM_SDK_STATIC_LIBRARIES - ${STEAM_SDK_STATIC_LIBRARIES}")
message ("STEAM_SDK - ${STEAM_SDK}")
