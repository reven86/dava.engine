if ( DAVA_FOUND )
    return ()
endif ()
set ( DAVA_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
add_subdirectory ( "${CURRENT_DIR}/../../Internal" ${CMAKE_CURRENT_BINARY_DIR}/DavaFramework )
set( DAVA_LIBRARY    "DavaFramework" )


