set(NAME_MODULE TArc)

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp)

set(INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/Sources")
set(FIND_PACKAGE QT5)

include_directories(${DAVA_INCLUDE_DIR})

setup_main_module()

if (QT5_FOUND)
  include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})
endif()
