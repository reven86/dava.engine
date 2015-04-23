cmake_minimum_required(VERSION 2.4)

# Settings:
set(CTEST_DASHBOARD_ROOT                "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CTestTest")
set(CTEST_SITE                          "BY1-WW-070")
set(CTEST_BUILD_NAME                    "CTestTest-Win32-MSBuild-Upload")

set(CTEST_SOURCE_DIRECTORY              "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CTestTestUpload")
set(CTEST_BINARY_DIRECTORY              "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CTestTestUpload")
set(CTEST_CMAKE_GENERATOR               "Visual Studio 14 2015")
set(CTEST_CMAKE_GENERATOR_PLATFORM      "")
set(CTEST_CMAKE_GENERATOR_TOOLSET       "")
set(CTEST_BUILD_CONFIGURATION           "$ENV{CMAKE_CONFIG_TYPE}")

CTEST_START(Experimental)
CTEST_CONFIGURE(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
CTEST_BUILD(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
CTEST_UPLOAD(FILES "${CTEST_SOURCE_DIRECTORY}/sleep.c" "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
CTEST_SUBMIT()
