
FIND_PROGRAM( SVN_BINARY svn)
IF(SVN_BINARY)
    EXECUTE_PROCESS(
        COMMAND ${SVN_BINARY} info ${CMAKE_SOURCE_DIR}
        COMMAND grep Revision
        COMMAND cut -f 2 -d " "
        COMMAND tr -d "\n"
        WORKING_DIRECTORY ${dir}
	RESULT_VARIABLE SVN_FOUND
	OUTPUT_VARIABLE SVN_WC_REVISION
    )
ENDIF(SVN_BINARY)

MACRO( SVN_CHECK ) 
    if( SVN_FOUND )

        if    ( MACOS )
            set( PREFIX "sh" )
        elseif( WIN32 )
            set( PREFIX ) 
        endif()

        add_custom_target (            
            CHECK_GIT ALL
            COMMAND ${PREFIX} ${CMAKE_BINARY_DIR}/SvnCheck.sh
          )

        set_target_properties( CHECK_GIT PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )

    endif()    
endmacro( GIT_CHECK )

if( SVN_FOUND )
    set( SUBVERSION_WC_REVISION ${SVN_WC_REVISION} )
    CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/SubVersions.in ${CMAKE_BINARY_DIR}/GitVersions )
    CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/SvnCheck.in  ${CMAKE_BINARY_DIR}/SvnCheck.sh @ONLY )
endif()

