

set( DAVA_LIBRARY    "DavaFramework" )

get_property( DAVA_IDX GLOBAL PROPERTY  DAVA_IDX  )

if( NOT DAVA_IDX )
    set_property( GLOBAL PROPERTY DAVA_IDX "1" )

else()    
    math( EXPR DAVA_IDX "${DAVA_IDX} + 1" )
    set( DAVA_LIBRARY    "DavaFramework_${DAVA_IDX}" )
    set_property( GLOBAL PROPERTY DAVA_IDX "${DAVA_IDX}" )
endif()

if( DavaFramework_FIND_COMPONENTS )
    append_property( DAVA_COMPONENTS "${DAVA_LIBRARY};${DavaFramework_FIND_COMPONENTS}" )
else()
    append_property( DAVA_COMPONENTS  "ALL"  )
endif()

set ( DAVA_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
add_subdirectory ( "${CURRENT_DIR}/../../Internal" ${CMAKE_CURRENT_BINARY_DIR}/DavaFramework )


