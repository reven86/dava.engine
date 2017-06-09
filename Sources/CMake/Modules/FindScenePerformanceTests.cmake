if ( SCENE_PERFORMANCE_TESTS_FOUND )
    return ()
endif ()
set ( SCENE_PERFORMANCE_TESTS_FOUND 1 )

include (GlobalVariables)

add_module_subdirectory ( ScenePerformanceTests "${DAVA_MODULES_DIR}/ScenePerformanceTests" )


