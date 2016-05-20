cmake_minimum_required(VERSION 3.4)
project(sqlite3)

include_directories(.)

set(SOURCE_FILES sqlite3.c)

add_library(sqlite3 STATIC ${SOURCE_FILES})