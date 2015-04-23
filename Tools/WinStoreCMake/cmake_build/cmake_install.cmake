# Install script for directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/CMake")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x86/Microsoft.VC140.CRT/msvcp140.dll")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Source/QtIFW/cmake.org.html")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/doc/cmake-3.2" TYPE FILE FILES "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Copyright.txt")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/cmake-3.2" TYPE DIRECTORY PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ DIR_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE FILES
    "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Help"
    "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Modules"
    "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Templates"
    REGEX "/[^/]*\\.sh[^/]*$" PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/KWIML/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Source/kwsys/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmzlib/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmcurl/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmcompress/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmbzip2/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmliblzma/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmlibarchive/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmexpat/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmjsoncpp/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Source/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/cmake_install.cmake")
  include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Auxiliary/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
