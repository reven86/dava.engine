set( DAVA_LIBRARY    "DavaFramework" )
if( DavaFramework_FIND_COMPONENTS )
    set( DAVA_COMPONENTS "${DAVA_LIBRARY};${DavaFramework_FIND_COMPONENTS}" )

else()
    set( DAVA_COMPONENTS "ALL" )

endif()

#####
list (FIND DAVA_COMPONENTS "DAVA_DISABLE_AUTOTESTS" _index)
if ( ${_index} GREATER -1 )
    set( DAVA_DISABLE_AUTOTESTS true )
endif()

list (FIND DAVA_COMPONENTS "DAVA_USE_RENDERSTATS" _index)
if ( ${_index} GREATER -1 )
    set( DAVA_USE_RENDERSTATS true )
endif()

list (FIND DAVA_COMPONENTS "DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME" _index)
if ( ${_index} GREATER -1 )
    set( DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME true )
endif()

list (FIND DAVA_COMPONENTS "DENY_RUN_MULTIPLE_APP_INSTANCES" _index)
if ( ${_index} GREATER -1 )
    set( DENY_RUN_MULTIPLE_APP_INSTANCES true )
endif()

list (FIND DAVA_COMPONENTS "ANDROID_USE_LOCAL_RESOURCES" _index)
if ( ${_index} GREATER -1 )
    set( ANDROID_USE_LOCAL_RESOURCES true )
endif()

list (FIND DAVA_COMPONENTS "DAVA_PLATFORM_QT" _index)
if ( ${_index} GREATER -1 )
    set( DAVA_PLATFORM_QT true )
endif()

#####

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