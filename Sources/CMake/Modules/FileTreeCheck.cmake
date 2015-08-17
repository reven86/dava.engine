MACRO ( FILE_TREE_CHECK arg_folders ) 
    find_package( PythonInterp   )

    if( PYTHONINTERP_FOUND AND NOT IGNORE_FILE_TREE_CHECK )
        set( TARGET_FILE_TREE_FOUND true )
        EXECUTE_PROCESS(
            COMMAND ${PYTHON_EXECUTABLE} "${DAVA_SCRIPTS_FILES_PATH}/file_tree_hash.py" ${arg_folders}
            OUTPUT_VARIABLE FILE_TREE_HASH
        )

        string(REPLACE "\n" "" FILE_TREE_HASH ${FILE_TREE_HASH})
        string(REPLACE ";" "," folders "${arg_folders}" )

        add_custom_target ( FILE_TREE_${PROJECT_NAME} ALL 
            COMMAND ${PYTHON_EXECUTABLE} ${DAVA_SCRIPTS_FILES_PATH}/versions_check.py ${CMAKE_CURRENT_BINARY_DIR} "${folders}" ${FILE_TREE_HASH}
        )

        set_target_properties( FILE_TREE_${PROJECT_NAME} PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )         

        FOREACH( item ${arg_folders} )
            message( " - ${item}" )        
        ENDFOREACH()

        message( "hash  ${FILE_TREE_HASH}" )        


    endif()    

endmacro( FILE_TREE_CHECK )
















