set(NAME_MODULE PhysicsDebug )

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE Sources/PhysicsDebug/*.h Sources/PhysicsDebug/*.hpp)
set(CPP_FILES_RECURSE Sources/PhysicsDebug/*.cpp)

set(PhysicsDebugRootDir "${CMAKE_CURRENT_SOURCE_DIR}")
set(PhysicsRootDir "${PhysicsDebugRootDir}/../Physics")

set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR}
                     "${PhysicsRootDir}/Libs/include/physx"
                     "${PhysicsRootDir}/Libs/include/PxShared"
                     "${PhysicsDebugRootDir}/Sources")
set(MODULE_INITIALIZATION_CODE Sources/PhysicsDebug/PhysicsDebugModule.h)

set(LibsPath "${PhysicsRootDir}/Libs")

set(WIN32_STATIC_DEBUG_LIBS "${LibsPath}/Win32/x86/PxPvdSDKDEBUG_x86.lib")

set(WIN32_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll"
                             "${LibsPath}/Win32/x86/PxPvdSDKDEBUG_x86.dll")

set(WIN32_STATIC_CHECKED_LIBS "${LibsPath}/Win32/x86/PxPvdSDKCHECKED_x86.lib")

set(WIN32_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll"
                               "${LibsPath}/Win32/x86/PxPvdSDKCHECKED_x86.dll")

set(WIN32_STATIC_PROFILE_LIBS "${LibsPath}/Win32/x86/PxPvdSDKPROFILE_x86.lib")

set(WIN32_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll"
                               "${LibsPath}/Win32/x86/PxPvdSDKPROFILE_x86.dll")

set(WIN64_STATIC_DEBUG_LIBS "${LibsPath}/Win32/x64/PxPvdSDKDEBUG_x64.lib")

set(WIN64_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll"
                             "${LibsPath}/Win32/x64/PxPvdSDKDEBUG_x64.dll")

set(WIN64_STATIC_CHECKED_LIBS "${LibsPath}/Win32/x64/PxPvdSDKCHECKED_x64.lib")

set(WIN64_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll"
                               "${LibsPath}/Win32/x64/PxPvdSDKCHECKED_x64.dll")

set(WIN64_STATIC_PROFILE_LIBS "${LibsPath}/Win32/x64/PxPvdSDKPROFILE_x64.lib")

set(WIN64_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll"
                               "${LibsPath}/Win32/x64/PxPvdSDKPROFILE_x64.dll")

set(WIN_UAP_STATIC_DEBUG_LIBS "${LibsPath}/Win10/$(Platform)/Debug/PxPvdSDKDEBUG_$(PlatformShortName).lib")
set(WIN_UAP_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win10/Win32/Debug/nvToolsExt32_1.dll"
                               "${LibsPath}/Win10/x64/Debug/PxPvdSDKDEBUG_x64.dll")

set(MACOS_STATIC_DEBUG_LIBS "${LibsPath}/MacOS/libPxPvdSDKDEBUG.a")
set(MACOS_STATIC_CHECKED_LIBS "${LibsPath}/MacOS/libPxPvdSDKCHECKED.a")
set(MACOS_STATIC_PROFILE_LIBS "${LibsPath}/MacOS/libPxPvdSDKPROFILE.a")
set(MACOS_STATIC_RELEASE_LIBS "${LibsPath}/MacOS/libPxPvdSDK.a")

set(IOS_STATIC_DEBUG_LIBS "${LibsPath}/iOS/libPxPvdSDKDEBUG.a")
set(IOS_STATIC_CHECKED_LIBS "${LibsPath}/iOS/libPxPvdSDKCHECKED.a")
set(IOS_STATIC_PROFILE_LIBS "${LibsPath}/iOS/libPxPvdSDKPROFILE.a")

set(ANDROID_STATIC_LIBS "${LibsPath}/Android/armeabi-v7a/libPxPvdSDK.a")

set(STATIC_LIBRARIES_WIN32_DEBUG "${WIN32_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN32_DEBUG "${WIN32_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_WIN64_DEBUG "${WIN64_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_DEBUG "${WIN64_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_WINUAP_DEBUG "${WIN_UAP_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WINUAP_DEBUG "${WIN_UAP_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_MACOS_RELEASE "${MACOS_STATIC_RELEASE_LIBS}")
set(STATIC_LIBRARIES_MACOS_DEBUG "${MACOS_STATIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_IOS_RELEASE "${IOS_STATIC_RELEASE_LIBS}")
set(STATIC_LIBRARIES_IOS_DEBUG "${IOS_STATIC_DEBUG_LIBS}")

set(BINARY_WIN32_DIR_RELEASE "${LibsPath}/Win32/x86")
set(BINARY_WIN32_DIR_RELWITHDEB "${LibsPath}/Win32/x86")
set(BINARY_WIN32_DIR_DEBUG "${LibsPath}/Win32/x86")
set(BINARY_WIN64_DIR_RELEASE "${LibsPath}/Win32/x64")
set(BINARY_WIN64_DIR_RELWITHDEB "${LibsPath}/Win32/x64")
set(BINARY_WIN64_DIR_DEBUG "${LibsPath}/Win32/x64")

setup_main_module()
