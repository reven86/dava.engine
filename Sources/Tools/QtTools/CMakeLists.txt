project( QtTools )

if( QT5_FOUND )
    include_directories( ${Qt5Core_INCLUDE_DIRS} )     
    include_directories( ${Qt5Gui_INCLUDE_DIRS} )
    include_directories( ${Qt5Widgets_INCLUDE_DIRS} )     
    include_directories( "QtTools" )                                      

    set( MOC_APP_SRCS )
    set( UI_APP_HDRS  )
    set( RCC_APP_SRCS )

    file              ( GLOB_RECURSE QT_LIST "*.h"  )
    qt5_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

    file              ( GLOB_RECURSE UI_LIST "*.ui" )
    qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

    file              ( GLOB_RECURSE RCC_LIST "*.qrc" )
    qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

    file( GLOB_RECURSE WIDGETS_SRC "*.h"  "*.cpp" )
    set( ADDED_SRC ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} ${WIDGETS_SRC} )
    set( Generated ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )

    generate_source_groups_project ( GROUP_FOLDERS Generated )
    add_library                    ( ${PROJECT_NAME} STATIC ${ADDED_SRC} )

else()
    message( "Warning: Qt is not found. Can not use QT_WIDGETS" )
 
endif()


