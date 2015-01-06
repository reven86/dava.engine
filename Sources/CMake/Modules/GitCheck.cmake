INCLUDE( FindGit   REQUIRED )

# get git revision
MACRO( GIT_WC_INFO  dir )
    EXECUTE_PROCESS(
        COMMAND
            ${GIT_EXECUTABLE} rev-parse --verify -q --short=7 HEAD
        WORKING_DIRECTORY
            ${dir}
        OUTPUT_VARIABLE
            GIT_WC_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endmacro( GIT_WC_INFO )

MACRO( GIT_CHECK ) 
    if( GIT_FOUND )

        if    ( MACOS )
            set( PREFIX "sh" )
        elseif( WIN32 )
            set( PREFIX ) 
        endif()

        add_custom_target (            
            CHECK_GIT ALL
            COMMAND ${PREFIX} ${CMAKE_BINARY_DIR}/GitCheck.sh
          )

        set_target_properties( CHECK_GIT PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )

    endif()    
endmacro( GIT_CHECK )

if( GIT_FOUND )
    GIT_WC_INFO(.)
    set( SUBVERSION_WC_REVISION ${GIT_WC_REVISION} )
    CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/SubVersions.in ${CMAKE_BINARY_DIR}/GitVersions )
    CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/GitCheck.in  ${CMAKE_BINARY_DIR}/GitCheck.sh @ONLY )
endif()

