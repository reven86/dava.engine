# CMake generated Testfile for 
# Source directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo
# Build directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/EnforceConfig.cmake")
add_test(SystemInformationNew "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/bin/cmake" "--system-information" "-G" "Visual Studio 14 2015")
subdirs(Utilities/KWIML)
subdirs(Source/kwsys)
subdirs(Utilities/cmzlib)
subdirs(Utilities/cmcurl)
subdirs(Utilities/cmcompress)
subdirs(Utilities/cmbzip2)
subdirs(Utilities/cmliblzma)
subdirs(Utilities/cmlibarchive)
subdirs(Utilities/cmexpat)
subdirs(Utilities/cmjsoncpp)
subdirs(Source)
subdirs(Utilities)
subdirs(Tests)
subdirs(Auxiliary)
