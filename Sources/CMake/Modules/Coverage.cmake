macro( coverage_processing  )  

if( MACOS AND COVERAGE )

    if( NOT COVERAGE_ARGS )
        set( COVERAGE_ARGS "''" )
    endif()

    string(REPLACE ";" " " DAVA_FOLDERS "${DAVA_FOLDERS}" )
    string(REPLACE "\"" "" DAVA_FOLDERS "${DAVA_FOLDERS}" )

    add_definitions( -DTEST_COVERAGE )
    add_definitions( -DDAVA_FOLDERS="${DAVA_FOLDERS}" )
    add_definitions( -DDAVA_UNITY_FOLDER="${CMAKE_CURRENT_BINARY_DIR}/unity_pack" )

    if( MAC_DISABLE_BUNDLE )
        set( APP_ATRIBUTE )
    else()
        set( APP_ATRIBUTE .app )
    endif()

    if( DEPLOY )
        set( EXECUT_FILE ${DEPLOY_DIR}/${PROJECT_NAME}${APP_ATRIBUTE})
    else()
        set( EXECUT_FILE ${CMAKE_BINARY_DIR}/$(CONFIGURATION)/${PROJECT_NAME}${APP_ATRIBUTE} )
    endif()

    set( COVERAGE_SCRIPT ${DAVA_ROOT_DIR}/RepoTools/coverage/coverage_report.py )

    set( COVERAGE_INDEX_HTML ${CMAKE_BINARY_DIR}/Coverage/index.html )
    set( COVERAGE_COMMAND_GENERATE_HTML "python ${COVERAGE_SCRIPT} --pathExecut ${EXECUT_FILE} --pathBuild ${CMAKE_BINARY_DIR} --pathReportOut ${CMAKE_BINARY_DIR}/Coverage --buildConfig $(CONFIGURATION) --runMode true --notExecute true --targetArgs ${COVERAGE_ARGS}" )

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/CoverageExecuteGenHtml.in
                    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CoverageExecuteGenHtml.cpp  )

    set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -D__DEBUG"   )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D__RELEASE" )

    add_executable( COVERAGE_${PROJECT_NAME} EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CoverageExecuteGenHtml.cpp  )
    add_dependencies( COVERAGE_${PROJECT_NAME}  ${PROJECT_NAME} )

    set_target_properties( COVERAGE_${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES YES )
    set_target_properties( COVERAGE_${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS YES )

    add_custom_command( TARGET COVERAGE_${PROJECT_NAME} 
            COMMAND ${PYTHON_EXECUTABLE} ${COVERAGE_SCRIPT}
                    --pathExecut    ${EXECUT_FILE}
                    --pathBuild     ${CMAKE_CURRENT_BINARY_DIR}
                    --pathReportOut ${CMAKE_CURRENT_BINARY_DIR}/Coverage
                    --buildConfig   $(CONFIGURATION)
                    --buildMode     true 
                    --targetArgs    ${COVERAGE_ARGS}
                    )


endif()


endmacro ()
