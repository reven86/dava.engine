if ( TARC_FOUND_FOUND )
    return ()
endif ()
set ( TARC_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "TArc"  )

add_subdirectory ( "${DAVA_MODULES_DIR}/TArc" ${CMAKE_CURRENT_BINARY_DIR}/TArc )
