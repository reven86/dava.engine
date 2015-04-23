cmake_minimum_required(VERSION 2.8.12)
set(CTEST_PROJECT_NAME "SmallAndFast")
set(CTEST_SOURCE_DIRECTORY "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CTestTest/SmallAndFast")
set(CTEST_BINARY_DIRECTORY "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CTestCoverageCollectGCOV")
set(CTEST_CMAKE_GENERATOR "Visual Studio 14 2015")
ctest_start(Experimental)
ctest_configure()
ctest_build()
ctest_test()

file(WRITE ${CTEST_BINARY_DIRECTORY}/CMakeFiles/echoargs.dir/echoargs.gcda
"dummy
")

include(CTestCoverageCollectGCOV)
set(tar_file ${CTEST_BINARY_DIRECTORY}/gcov.tar)
ctest_coverage_collect_gcov(
  TARBALL "${tar_file}"
  SOURCE "${CTEST_SOURCE_DIRECTORY}"
  BUILD "${CTEST_BINARY_DIRECTORY}"
  GCOV_COMMAND "${CMAKE_COMMAND}"
  GCOV_OPTIONS -P "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CTestCoverageCollectGCOV/fakegcov.cmake")

execute_process(COMMAND
      ${CMAKE_COMMAND} -E tar tf ${tar_file}
      OUTPUT_VARIABLE out
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

set(expected_out
"Testing/CoverageInfo/echoargs.gcov
Testing/CoverageInfo/data.json
CMakeFiles/echoargs.dir/Labels.json
")

if("${out}" STREQUAL "${expected_out}")
  message("PASSED with correct output: ${out}")
else()
  message(FATAL_ERROR "FAILED: expected:\n${expected_out}\nGot:\n${out}")
endif()
