if ( QT_HELPERS_FOUND )
    return ()
endif ()
set ( QT_HELPERS_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "QtHelpers"  )

add_subdirectory ( "${DAVA_MODULES_DIR}/QtHelpers" ${CMAKE_CURRENT_BINARY_DIR}/QtHelpers )
