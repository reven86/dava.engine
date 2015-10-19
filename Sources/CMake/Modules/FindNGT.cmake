if (NGT_FOUND)
    return()
endif()

if (NOT NGT_ROOT)
    message(FATAL_ERROR "marco define_ngt_pathes have not been called")
endif()

set (NGT_FOUND 1)

include (NGTMacro)

get_filename_component(NGT_ABS_CORE_PATH ${NGT_CORE_PATH} ABSOLUTE)
get_filename_component(NGT_ABS_SRC_ROOT ${NGT_SRC_ROOT} ABSOLUTE)

include_directories(${NGT_ABS_CORE_PATH})
include_directories(${NGT_ABS_CORE_PATH}/interfaces)
include_directories(${NGT_ABS_SRC_ROOT})
include_directories(${NGT_ABS_SRC_ROOT}/../)
get_subdirs_list(SUBDIRS ${NGT_ABS_CORE_PATH})

foreach(SUBDIR ${SUBDIRS})
    set(LIB_NAME ${SUBDIR})
    define_ngt_lib(${NGT_ABS_CORE_PATH}/${SUBDIR} ${LIB_NAME})
endforeach()

get_filename_component(NGT_ABS_PLUGINS_PATH ${NGT_PLUGINS_PATH} ABSOLUTE)
get_subdirs_list(SUBDIRS ${NGT_ABS_PLUGINS_PATH})

foreach(SUBDIR ${SUBDIRS})
    set(LIB_NAME ${SUBDIR})
    define_ngt_lib(${NGT_ABS_PLUGINS_PATH}/${SUBDIR} ${LIB_NAME})
endforeach()

set(CMAKE_MODULE_TMP ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${NGT_ROOT}/build/cmake/)

include (BWMacros)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_TMP})
