include ( GlobalVariables      )

FIND_PROGRAM( PYTHON_BINARY python )

MACRO( FILE_TREE_CHECK folders ) 

    if( PYTHON_BINARY AND NOT IGNORE_FILE_TREE_CHECK AND NOT ANDROID )

        EXECUTE_PROCESS(
            COMMAND ${PYTHON_BINARY} "${DAVA_SCRIPTS_FILES_PATH}/FileTreeHash.py" ${folders}
            OUTPUT_VARIABLE FILE_TREE_HASH
        )

        string(REPLACE "\n" "" FILE_TREE_HASH ${FILE_TREE_HASH})

        add_custom_target ( FILE_TREE ALL 
            COMMAND python ${DAVA_SCRIPTS_FILES_PATH}/VersionsCheck.py ${CMAKE_CURRENT_BINARY_DIR} "${folders}" ${FILE_TREE_HASH}
        )

        set_target_properties( FILE_TREE PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )         

        FOREACH( item ${folders} )
            message( " - ${item}" )        
        ENDFOREACH()

        message( "hash  ${FILE_TREE_HASH}" )        
       
    endif()    

endmacro( FILE_TREE_CHECK )
















