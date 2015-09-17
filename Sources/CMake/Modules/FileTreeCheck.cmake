set( TARGET_FILE_TREE_FOUND false )

MACRO ( FILE_TREE_CHECK arg_folders ) 
    find_package( PythonInterp   )

    if( PYTHONINTERP_FOUND AND NOT IGNORE_FILE_TREE_CHECK )
        set( TARGET_FILE_TREE_FOUND true PARENT_SCOPE )

        string(REPLACE ";" " " folders "${arg_folders}" )
        string(REPLACE "\"" "" folders "${arg_folders}" )

        EXECUTE_PROCESS(
            COMMAND ${PYTHON_EXECUTABLE} "${DAVA_SCRIPTS_FILES_PATH}/file_tree_hash.py" ${folders}
            OUTPUT_VARIABLE FILE_TREE_HASH
        )

        string(REPLACE "\n" "" FILE_TREE_HASH ${FILE_TREE_HASH})

        add_custom_target ( FILE_TREE_${PROJECT_NAME} ALL 
            COMMAND ${PYTHON_EXECUTABLE} ${DAVA_SCRIPTS_FILES_PATH}/versions_check.py ${CMAKE_CURRENT_BINARY_DIR} ${FILE_TREE_HASH} ${folders}
        )

        set_target_properties( FILE_TREE_${PROJECT_NAME} PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )         

        FOREACH( item ${arg_folders} )
            message( " - ${item}" )        
        ENDFOREACH()

        message( "hash  ${FILE_TREE_HASH}" )        


    endif()    

endmacro( FILE_TREE_CHECK )
















