cmake_minimum_required(VERSION 3.4)
project(unibreak)

include_directories(./src)

FILE(GLOB SOURCE_FILES src/*.c)
add_library(unibreak STATIC ${SOURCE_FILES})

if(WIN32)
  set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd" ) 
  set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  message( FATAL_ERROR "This CMake file is only supported on win32 - use autotools on macOS and linux" )
endif()
