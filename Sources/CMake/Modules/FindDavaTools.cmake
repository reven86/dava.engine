if ( DAVA_TOOLS_FOUND )
    return ()
endif ()
set ( DAVA_TOOLS_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
add_subdirectory ( "${CURRENT_DIR}/../../Tools" ${CMAKE_CURRENT_BINARY_DIR}/DavaTools )
set( DAVA_TOOLS_LIBRARY    DavaTools )


