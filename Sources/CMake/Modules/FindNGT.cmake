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

if ( WIN32 )
  if (X64_MODE)
      ASSERT(QT5_PATH_WIN64 "Qt path on windows not specified")
      set(Qt5_DIR ${QT5_PATH_WIN64})
  else()
      ASSERT(QT5_PATH_WIN "Qt path on windows not specified")
      set(Qt5_DIR ${QT5_PATH_WIN})
  endif()
elseif (MACOS)
  ASSERT(QT5_PATH_MAC "Qt path on macos not specified")
  set(Qt5_DIR ${QT5_PATH_MAC})
else()
  ASSERT(NOT_SUPPORTED_PLATFORM "NGT not support this platform")
endif()

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
