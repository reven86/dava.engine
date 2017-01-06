set( DAVA_LIBRARY    "DavaFramework" )
if( DavaFramework_FIND_COMPONENTS )
    set( DAVA_COMPONENTS "${DAVA_LIBRARY};${DavaFramework_FIND_COMPONENTS}" )

else()
    set( DAVA_COMPONENTS "ALL" )

endif()

set ( DAVA_FOUND 1 )

get_filename_component( CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE )

add_module_subdirectory( ${DAVA_LIBRARY}  "${CURRENT_DIR}/../../Internal"
                         COMPONENTS  "${DAVA_COMPONENTS}" )

get_property( MODULES_NAME GLOBAL PROPERTY  MODULES_NAME )
set( DAVA_LIBRARY ${MODULES_NAME})
set( PACKAGE_${NAME}_STATIC_LIBRARIES ${MODULES_NAME} )

if( APPLE )
    set_target_properties(${DAVA_LIBRARY} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")
endif()