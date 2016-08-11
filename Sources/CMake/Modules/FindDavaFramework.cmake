

set( DAVA_LIBRARY    "DavaFramework" )


if( DavaFramework_FIND_COMPONENTS )
    append_property( DAVA_COMPONENTS "${DAVA_LIBRARY};${DavaFramework_FIND_COMPONENTS}" )
else()
    append_property( DAVA_COMPONENTS  "ALL"  )
endif()

set ( DAVA_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
add_subdirectory ( "${CURRENT_DIR}/../../Internal" ${CMAKE_CURRENT_BINARY_DIR}/DavaFramework )


