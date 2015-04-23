if (NOT TEST_SOURCE)
  set(TEST_SOURCE "${TEST}")
endif ()

set(source_dir "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CMakeOnly/${TEST_SOURCE}")
set(binary_dir "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeOnly/${TEST}-build")
file(REMOVE_RECURSE "${binary_dir}")
file(MAKE_DIRECTORY "${binary_dir}")
execute_process(
  COMMAND  ${CMAKE_COMMAND} ${CMAKE_ARGS}
  "${source_dir}" -G "Visual Studio 14 2015"
  -A ""
  -T ""
  WORKING_DIRECTORY "${binary_dir}"
  RESULT_VARIABLE result
  )
if(result)
  message(FATAL_ERROR "CMake failed to configure ${TEST}")
endif()
