if ( QT_TOOLS_FOUND )
    return ()
endif ()
set ( QT_TOOLS_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "QtTools"  )

add_subdirectory ( "${DAVA_MODULES_DIR}/QtTools" ${CMAKE_CURRENT_BINARY_DIR}/QtTools )
