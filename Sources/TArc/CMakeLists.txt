set(NAME_MODULE TArc)

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp)

set(INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}")
set (FIND_SYSTEM_LIBRARY QT5)

include_directories(${DAVA_INCLUDE_DIR})
include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})

setup_main_module()
