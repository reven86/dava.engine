include ( GlobalVariables      )

FIND_PROGRAM( PYTHON_BINARY python )

MACRO( FILE_TREE_CHECK folders ) 

    if( PYTHON_BINARY AND NOT IGNORE_FILE_TREE_CHECK AND NOT ANDROID )

        if( APPLE )
            set( SH_PREFIX "sh" )

        elseif( WIN32 )
            set( SH_PREFIX ) 

        endif()

        EXECUTE_PROCESS(
            COMMAND ${PYTHON_BINARY} "${DAVA_SCRIPTS_FILES_PATH}/FileTreeHash.py" ${folders}
            OUTPUT_VARIABLE FILE_TREE_HASH
        )
        
        set( GET_VERSIONS_COMMAND "$(python ${DAVA_SCRIPTS_FILES_PATH}/FileTreeHash.py $1)" )
        set( CURRENT_VERSIONS    "$2"   )

        configure_file( ${DAVA_CONFIGURE_FILES_PATH}/VersionsCheck.in ${CMAKE_BINARY_DIR}/VersionsCheck.sh @ONLY )
        
        add_custom_target ( FILE_TREE ALL 
            COMMAND ${SH_PREFIX} ${CMAKE_BINARY_DIR}/VersionsCheck.sh '${folders}' ${FILE_TREE_HASH}
        )
        set_target_properties( FILE_TREE PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )         

        FOREACH( item ${folders} )
            message( " - ${item}" )        
        ENDFOREACH()

        message( "hash  ${FILE_TREE_HASH}" )        
       
    endif()    

endmacro( FILE_TREE_CHECK )
















