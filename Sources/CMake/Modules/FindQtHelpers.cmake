if ( QT_HELPERS_FOUND )
    return ()
endif ()
set ( QT_HELPERS_FOUND 1 )

append_property( DAVA_COMPONENTS  "QtHelpers"  )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )
add_subdirectory ( "${CURRENT_DIR}/../../Tools/QtHelpers" ${CMAKE_CURRENT_BINARY_DIR}/QtHelpers )
