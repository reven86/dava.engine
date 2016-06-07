cmake_minimum_required(VERSION 3.4)
project(double_commander_packer_plugin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static -O3")

set(SOURCE_FILES main.cpp pack_archive.cpp pack_archive.cpp lz4.c
        lz4.h pack_archive.h pack_format.h wcxhead.h)

add_library(double_commander_packer_plugin SHARED ${SOURCE_FILES})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(OUT_PREFIX "_win.wcx")
else()
    set(OUT_PREFIX "_mac.wcx")
endif()

set_target_properties(double_commander_packer_plugin PROPERTIES OUTPUT_NAME "packer_plugin"
        SUFFIX ${OUT_PREFIX})

add_custom_command(TARGET double_commander_packer_plugin
        POST_BUILD
        COMMAND "strip.exe" "libpacker_plugin${OUT_PREFIX}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")