include ( GlobalVariables      )

if (NGT_FOUND)
    return()
endif()
set(NGT_FOUND 1)

if (NOT NGT_PATH)
    message(FATAL_ERROR "Please set the correct path to NGT in file DavaConfig.in")
endif()

get_filename_component(NGT_ROOT_DIR ${NGT_PATH} ABSOLUTE)
set(BW_CMAKE_TARGET DAVA)
set(Qt5_DIR ${QT5_PATH_WIN})

INCLUDE_DIRECTORIES( ${NGT_ROOT_DIR}/src/core/lib )
add_subdirectory(${NGT_ROOT_DIR}/src ngt/)
