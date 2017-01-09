if ( QT_HELPERS_FOUND )
    return ()
endif ()
set ( QT_HELPERS_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory( QtHelpers  "${DAVA_MODULES_DIR}/QtHelpers" )