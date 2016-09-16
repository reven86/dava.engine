set(NAME_MODULE QtHelpers)

set(MODULE_TYPE STATIC)

set( MOC_APP_SRCS )
set( UI_APP_HDRS  )
set( RCC_APP_SRCS )

file              ( GLOB_RECURSE UI_LIST "${CMAKE_CURRENT_LIST_DIR}/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB_RECURSE RCC_LIST "${CMAKE_CURRENT_LIST_DIR}/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp *.mm)
set(HPP_FILES ${UI_APP_HDRS})
set(CPP_FILES ${RCC_APP_SRCS})

set (Generated ${UI_APP_HDRS} ${RCC_APP_SRCS})
set(GROUP_SOURCE Generated)

set(INCLUDES "${CMAKE_CURRENT_LIST_DIR}")
set(FIND_PACKAGE QT5)
set(EXCLUDE_FROM_ALL 1)

set(INCLUDES_PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR})

setup_main_module()
