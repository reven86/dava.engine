if ( TARC_FOUND_FOUND )
    return ()
endif ()
set ( TARC_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory( TArc  "${DAVA_MODULES_DIR}/TArc" )