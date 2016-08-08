set(NAME_MODULE TArc)

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp)

set( INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}" )

find_package(QT5 REQUIRED)

include_directories ( ${DAVA_INCLUDE_DIR} ) 

setup_main_module()
