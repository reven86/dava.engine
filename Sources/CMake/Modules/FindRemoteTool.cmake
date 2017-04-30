if ( REMOTE_TOOL_FOUND )
    return ()
endif ()
set ( REMOTE_TOOL_FOUND 1 )

add_definitions(-DWITH_SCENE_PERFORMANCE_TESTS)

include (GlobalVariables)

add_module_subdirectory ( RemoteTool "${DAVA_MODULES_DIR}/RemoteTool" )
