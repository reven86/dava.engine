project( QtTools )

find_package( QT5 REQUIRED )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( "QtTools" )

set( MOC_APP_SRCS )
set( UI_APP_HDRS  )
set( RCC_APP_SRCS )

file              ( GLOB_RECURSE UI_LIST "*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB_RECURSE RCC_LIST "*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

file( GLOB_RECURSE WIDGETS_SRC "*.h"  "*.cpp" "*.mm")
set( ADDED_SRC ${UI_APP_HDRS} ${RCC_APP_SRCS} ${WIDGETS_SRC} )
set( Generated ${UI_APP_HDRS} ${RCC_APP_SRCS} )

set( UNITY_IGNORE_LIST_APPLE *.mm)

generated_unity_sources        ( ADDED_SRC )

generate_source_groups_project ( GROUP_FOLDERS Generated )
add_library                    ( ${PROJECT_NAME} STATIC ${ADDED_SRC} )



