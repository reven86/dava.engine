# CMake generated Testfile for 
# Source directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CMakeLib
# Build directory: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeLib
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(CMakeLib.testGeneratedFileStream "CMakeLibTests" "testGeneratedFileStream")
add_test(CMakeLib.testRST "CMakeLibTests" "testRST" "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CMakeLib")
add_test(CMakeLib.testSystemTools "CMakeLibTests" "testSystemTools")
add_test(CMakeLib.testUTF8 "CMakeLibTests" "testUTF8")
add_test(CMakeLib.testXMLParser "CMakeLibTests" "testXMLParser")
add_test(CMakeLib.testXMLSafe "CMakeLibTests" "testXMLSafe")
add_test(CMakeLib.testVisualStudioSlnParser "CMakeLibTests" "testVisualStudioSlnParser")
subdirs(PseudoMemcheck)
