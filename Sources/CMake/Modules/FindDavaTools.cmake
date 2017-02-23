if ( DAVA_TOOLS_FOUND )
    return ()
endif ()
set ( DAVA_TOOLS_FOUND 1 )
include (GlobalVariables)

add_module_subdirectory( Tools  "${DAVA_MODULES_DIR}/Tools" )
