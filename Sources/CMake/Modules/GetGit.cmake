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

if( GIT_FOUND )
    GIT_WC_INFO(.)
    CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles/GitVersions.in  ${CMAKE_BINARY_DIR}/GitVersions )

endif()

