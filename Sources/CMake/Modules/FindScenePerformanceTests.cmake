if ( SCENE_PERFORMANCE_TESTS_FOUND )
    return ()
endif ()
set ( SCENE_PERFORMANCE_TESTS_FOUND 1 )

include (GlobalVariables)
append_property( DAVA_COMPONENTS  "ScenePerformanceTests" )

add_subdirectory ( "${DAVA_MODULES_DIR}/ScenePerformanceTests" ${CMAKE_CURRENT_BINARY_DIR}/ScenePerformanceTests )
