if ( PERFORMANCE_TESTS_FOUND )
    return ()
endif ()
set ( PERFORMANCE_TESTS_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "PerformanceTests" )

add_subdirectory ( "${DAVA_MODULES_DIR}/PerformanceTests" ${CMAKE_CURRENT_BINARY_DIR}/PerformanceTests )
