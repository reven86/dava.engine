set(NAME_MODULE TArc)

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE Sources/TArc/*.h Sources/TArc/*.hpp)
set(CPP_FILES_RECURSE Sources/TArc/*.cpp)
set(CPP_FILES_RECURSE_MACOS Sources/TArc/*.mm)
set(CUSTOM_PACK_1 "Selftests")

set(TArcRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set(INCLUDES "${TArcRootDir}/Sources" "${DAVA_THIRD_PARTY_ROOT_PATH}/include/googlemock")
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set (libsPath "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake")

set(STATIC_LIBRARIES_MACOS_RELEASE "${libsPath}/mac/libgmock.a")
set(STATIC_LIBRARIES_MACOS_DEBUG "${libsPath}/mac/libgmock.a")

set(STATIC_LIBRARIES_WIN32_RELEASE "${libsPath}/win/x86/Release/gmock.lib")
set(STATIC_LIBRARIES_WIN32_DEBUG "${libsPath}/win/x86/Debug/gmock.lib")

set(STATIC_LIBRARIES_WIN64_RELEASE "${libsPath}/win/x64/Release/gmock.lib")
set(STATIC_LIBRARIES_WIN64_DEBUG "${libsPath}/win/x64/Debug/gmock.lib")

set(QT_RES_FILES "Sources/TArc/Controls/*.qrc")
set(QT_UI_FILES "Sources/TArc/Controls/ColorPicker/Private/Resources/*.ui")

set(FIND_PACKAGE QT5)
set(PACKAGE_QT5_INCLUDES Qt5Widgets_PRIVATE_INCLUDE_DIRS)

setup_main_module()

find_package( PythonInterp )
if( NOT PYTHONINTERP_FOUND )
    set( PYTHON_EXECUTABLE python )
endif()

EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} "${TArcRootDir}/Sources/TArc/Testing/Private/DeadCodeTrickGen.py")

