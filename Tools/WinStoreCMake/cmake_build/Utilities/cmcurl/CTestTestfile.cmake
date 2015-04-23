# CMake generated Testfile for 
# Source directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Utilities/cmcurl
# Build directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Utilities/cmcurl
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(curl "LIBCURL" "http://open.cdash.org/user.php")
subdirs(lib)
