cmake_minimum_required (VERSION 3.0)

project( dep )

set (CMAKE_CXX_FLAGS "-std=c++14")
add_executable( ${PROJECT_NAME} dep.cpp )
