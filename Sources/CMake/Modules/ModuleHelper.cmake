
 
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
    reset_MAIN_MODULE_VALUES() 
    add_subdirectory ( ${SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${NAME} )

    append_property( PLUGIN_LIST ${NAME} )

    if( DEPLOY )
        set( OUT_PLUGIN_DIR ${DEPLOY_EXECUTE_DIR} )

        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
            set_target_properties( ${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUT_PLUGIN_DIR} )
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
        
    else()
        set( OUT_PLUGIN_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>" )
        set_target_properties( ${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${OUT_PLUGIN_DIR} )
    endif()


    reset_MAIN_MODULE_VALUES() 

endmacro()
