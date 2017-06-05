cmake_minimum_required(VERSION 3.4)

project(mongodb)
set (CMAKE_CXX_STANDARD 11)
include_directories("${PROJECT_SOURCE_DIR}/include")

set (SOURCE_FILES
    src/bson.c
    src/encoding.c
    src/gridfs.c
    src/md5.c
    src/mongo.c
    src/numbers.c
)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    list (APPEND SOURCE_FILES
        "src/env_win32.c"
    )
elseif (UNIX)
    list (APPEND SOURCE_FILES
        "src/env_posix.c"
        "src/env_standard.c"
    )
endif()

add_library("${PROJECT_NAME}" STATIC ${SOURCE_FILES} ) 
