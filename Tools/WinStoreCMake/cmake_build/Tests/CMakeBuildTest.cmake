# create the binary directory
make_directory("E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeBuildCOnly")

# remove the CMakeCache.txt file from the source dir
# if there is one, so that in-source cmake tests
# still pass
message("Remove: E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/COnly/CMakeCache.txt")
file(REMOVE "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/COnly/CMakeCache.txt")

# run cmake in the binary directory
message("running: ${CMAKE_COMMAND}")
execute_process(COMMAND "${CMAKE_COMMAND}"
  "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/COnly"
  "-GVisual Studio 14 2015"
  -A ""
  -T ""
  WORKING_DIRECTORY "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeBuildCOnly"
  RESULT_VARIABLE RESULT)
if(RESULT)
  message(FATAL_ERROR "Error running cmake command")
endif()

# Now use the --build option to build the project
message("running: ${CMAKE_COMMAND} --build")
execute_process(COMMAND "${CMAKE_COMMAND}"
  --build "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeBuildCOnly" --config Debug
  RESULT_VARIABLE RESULT)
if(RESULT)
  message(FATAL_ERROR "Error running cmake --build")
endif()

# check for configuration types
set(CMAKE_CONFIGURATION_TYPES Debug;Release;MinSizeRel;RelWithDebInfo)
# run the executable out of the Debug directory if there
# are configuration types
if(CMAKE_CONFIGURATION_TYPES)
  set(RUN_TEST "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeBuildCOnly/Debug/COnly")
else()
  set(RUN_TEST "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeBuildCOnly/COnly")
endif()
# run the test results
message("running [${RUN_TEST}]")
execute_process(COMMAND "${RUN_TEST}" RESULT_VARIABLE RESULT)
if(RESULT)
  message(FATAL_ERROR "Error running test COnly")
endif()

# build it again with clean and only COnly target
execute_process(COMMAND "${CMAKE_COMMAND}"
  --build "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeBuildCOnly" --config Debug
  --clean-first --target COnly
  RESULT_VARIABLE RESULT)
if(RESULT)
  message(FATAL_ERROR "Error running cmake --build")
endif()

# run it again after clean
execute_process(COMMAND "${RUN_TEST}" RESULT_VARIABLE RESULT)
if(RESULT)
  message(FATAL_ERROR "Error running test COnly after clean ")
endif()
