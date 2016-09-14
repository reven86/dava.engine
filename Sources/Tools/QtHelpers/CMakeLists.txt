set(NAME_MODULE QtHelpers)

set(MODULE_TYPE STATIC)

set( MOC_APP_SRCS )
set( UI_APP_HDRS  )
set( RCC_APP_SRCS )

file              ( GLOB_RECURSE UI_LIST "*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB_RECURSE RCC_LIST "*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

set( ADDED_SRC ${UI_APP_HDRS} ${RCC_APP_SRCS} ${WIDGETS_SRC} )

set(UNITY_IGNORE_LIST_APPLE *.mm)
set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp *.mm ${ADDED_SRC})

set(INCLUDES "${CMAKE_CURRENT_LIST_DIR}")
set(FIND_PACKAGE QT5)
set(EXCLUDE_FROM_ALL 1)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_LIST_DIR} )

setup_main_module()
