if ( REMOTE_TOOL_FOUND )
    return ()
endif ()
set ( REMOTE_TOOL_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "RemoteTool" )

add_module_subdirectory ( RemoteTool "${DAVA_MODULES_DIR}/RemoteTool" )