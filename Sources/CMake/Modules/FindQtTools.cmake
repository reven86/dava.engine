if ( QT_TOOLS_FOUND )
    return ()
endif ()
set ( QT_TOOLS_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory( QtTools  "${DAVA_MODULES_DIR}/QtTools" )