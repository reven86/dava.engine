if ( DAVA_TOOLS_FOUND )
    return ()
endif ()
set ( DAVA_TOOLS_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )

add_module_subdirectory( DavaTools  "${CURRENT_DIR}/../../Tools" )

include_directories( "${CURRENT_DIR}/../../Tools" )
set( DAVA_TOOLS_LIBRARY    DavaTools )
