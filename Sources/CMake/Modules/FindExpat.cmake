if (EXPAT_FOUND)
    return()
endif()

set(EXPAT_FOUND 1)

include (CMake-common)
include (NGTMacro)
add_subdirectory(${NGT_SRC_ROOT}/third_party/expat expat)
