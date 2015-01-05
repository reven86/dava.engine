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

MACRO( CHECK_GIT ) 
    if( WIN32 AND GIT_FOUND )
        add_custom_target (            
            CHECK_GIT ALL
            COMMAND for /f %%x in (${CMAKE_BINARY_DIR}/GitVersions) do (set GIT_CURRENT_VERSIONS=%%x)
            COMMAND FOR /F "usebackq" %%w IN (`git rev-parse --verify -q --short HEAD`) DO SET GIT_VERSIONS=%%w
            COMMAND echo "tyertyertyer--- %GIT_VERSIONS% --  %GIT_CURRENT_VERSIONS%"
            COMMAND if NOT %GIT_VERSIONS% == %GIT_CURRENT_VERSIONS%  touch ${CMAKE_CURRENT_LIST_FILE}
         )
        set_target_properties( CHECK_GIT PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )

    endif()

    if( MACOS AND GIT_FOUND )
        add_custom_target (            
            CHECK_GIT ALL
            COMMAND sh ${CMAKE_BINARY_DIR}/XCodeCheckGit.sh
          )
        set_target_properties( CHECK_GIT PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )

    endif()    
endmacro( CHECK_GIT )

if( GIT_FOUND )
    GIT_WC_INFO(.)
    CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/GitVersions.in    ${CMAKE_BINARY_DIR}/GitVersions )

    if( MACOS )
        CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/XCodeCheckGit.in  ${CMAKE_BINARY_DIR}/XCodeCheckGit.sh @ONLY )
    endif()

endif()

