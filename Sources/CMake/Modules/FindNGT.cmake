if (NGT_FOUND)
    return()
endif()

if (NOT NGT_ROOT)
    message(FATAL_ERROR "marco define_ngt_pathes have not been called")
endif()

set (NGT_FOUND 1)
set (DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME 1 CACHE INTERNAL "")
include (CMake-common)
include (NGTMacro)

set(Qt5_DIR ${QT_ACTUAL_PATH})

append_ngt_includes()
get_subdirs_list(SUBDIRS ${NGT_CORE_PATH})

foreach(SUBDIR ${SUBDIRS})
    set(LIB_NAME ${SUBDIR})
    define_ngt_lib(${NGT_CORE_PATH}/${SUBDIR} ${LIB_NAME})
endforeach()

get_subdirs_list(SUBDIRS ${NGT_PLUGINS_PATH})

foreach(SUBDIR ${SUBDIRS})
    set(LIB_NAME ${SUBDIR})
    define_ngt_lib(${NGT_PLUGINS_PATH}/${SUBDIR} ${LIB_NAME})
endforeach()

set(CMAKE_MODULE_TMP ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${NGT_ROOT}/build/cmake/)

include (BWQtCommon)
include (BWMacros)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_TMP})
