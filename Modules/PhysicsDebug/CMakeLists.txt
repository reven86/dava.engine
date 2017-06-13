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

set(WIN32_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll")
set(WIN32_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll")
set(WIN32_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll")

set(WIN64_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll")
set(WIN64_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll")
set(WIN64_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll")

set(DYNAMIC_LIBRARIES_WIN32_DEBUG "${WIN32_DYNAMIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_DEBUG "${WIN64_DYNAMIC_DEBUG_LIBS}")

setup_main_module()
