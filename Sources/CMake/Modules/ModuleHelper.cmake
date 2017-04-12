include ( CMakeParseArguments  )
 
macro ( add_module_subdirectory NAME SOURCE_DIR )
    cmake_parse_arguments ( ARG ""  "" "COMPONENTS" ${ARGN} )

    if( ARG_COMPONENTS )
        set( MODULE_COMPONENTS_VALUE_NAME ${NAME} )
    else()
        set( MODULE_COMPONENTS_VALUE_NAME ) 
    endif()

    set_property( GLOBAL PROPERTY COMPONENTS_${MODULE_COMPONENTS_VALUE_NAME} ${ARG_COMPONENTS} )

    foreach( VALUE ${MAIN_MODULE_VALUES} )
        set( ${VALUE}_DIR_NAME ${${VALUE}} )
        set( ${VALUE})
    endforeach()
    
    add_subdirectory ( ${SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${NAME} )

    foreach( VALUE ${MAIN_MODULE_VALUES} )
        set(  ${VALUE} ${${VALUE}_DIR_NAME} )
    endforeach()

endmacro()    


macro ( add_plugin NAME SOURCE_DIR )
    cmake_parse_arguments ( ARG ""  "" "COMPONENTS" ${ARGN} )
    if( ARG_COMPONENTS )
        set( MODULE_COMPONENTS_VALUE_NAME ${NAME} )
    else()
        set( MODULE_COMPONENTS_VALUE_NAME ) 
    endif()

    set_property( GLOBAL PROPERTY COMPONENTS_${MODULE_COMPONENTS_VALUE_NAME} ${ARG_COMPONENTS} ${NAME} )

    foreach( VALUE ${GLOBAL_PROPERTY_VALUES} )
        set( ${VALUE} )
        set_property( GLOBAL PROPERTY ${VALUE} ${${VALUE}} )
    endforeach()

    add_subdirectory ( ${SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${NAME} )

    foreach( VALUE ${GLOBAL_PROPERTY_VALUES} )
        set( ${VALUE} )
        set_property( GLOBAL PROPERTY ${VALUE} ${${VALUE}} )
    endforeach()
    
endmacro()
