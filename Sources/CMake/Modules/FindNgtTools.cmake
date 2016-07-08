if ( DAVA_NGTTOOLS_FOUND )
    return ()
endif ()
set ( DAVA_NGTTOOLS_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
add_subdirectory ( "${CURRENT_DIR}/../../Tools/NgtTools" ${CMAKE_CURRENT_BINARY_DIR}/NgtTools )
