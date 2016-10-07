set(NAME_MODULE TArc)

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE Sources/*.h Sources/*.hpp)
set(CPP_FILES_RECURSE Sources/*.cpp)
set(CPP_FILES_RECURSE_MACOS Sources/*.mm)
set(CUSTOM_PACK_1 "Selftests")

set(TArcRootDir "${CMAKE_CURRENT_SOURCE_DIR}")
set(TArcLibsDir "${TArcRootDir}/Libs")

set(INCLUDES "${TArcRootDir}/Sources" "${TArcLibsDir}/Include")
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set(STATIC_LIBRARIES_MACOS_RELEASE "${TArcLibsDir}/Mac/Release/libgmock.a")
set(STATIC_LIBRARIES_MACOS_DEBUG "${TArcLibsDir}/Mac/Debug/libgmock.a")

set(STATIC_LIBRARIES_WIN32_RELEASE "${TArcLibsDir}/Win32/x86/Release/gmock.lib")
set(STATIC_LIBRARIES_WIN32_DEBUG "${TArcLibsDir}/Win32/x86/Debug/gmock.lib")

set(STATIC_LIBRARIES_WIN64_RELEASE "${TArcLibsDir}/Win32/x64/Release/gmock.lib")
set(STATIC_LIBRARIES_WIN64_DEBUG "${TArcLibsDir}/Win32/x64/Debug/gmock.lib")

set(FIND_PACKAGE QT5)
set(EXCLUDE_FROM_ALL 1)
set(PACKAGE_QT5_INCLUDES Qt5Core_PRIVATE_INCLUDE_DIRS)

setup_main_module()
