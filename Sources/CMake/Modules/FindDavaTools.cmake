if ( DAVA_TOOLS_FOUND )
    return ()
endif ()
set ( DAVA_TOOLS_FOUND 1 )
include ( ModuleHelper  )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )

add_module_subdirectory(DavaTools  "${CURRENT_DIR}/../../../Sources/Tools")

include_directories( "${CURRENT_DIR}/../../../Sources/Tools" )

set( DAVA_TOOLS_LIBRARY    DavaTools )
