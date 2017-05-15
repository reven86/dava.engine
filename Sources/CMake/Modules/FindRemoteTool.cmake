if ( REMOTE_TOOL_FOUND )
    return ()
endif ()
set ( REMOTE_TOOL_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory ( RemoteTool "${DAVA_MODULES_DIR}/RemoteTool" )
