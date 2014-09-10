
set( INCLUDES_DIR  ${INCLUDES_DIR} "${PROJECT_SOURCE_DIR}/../../Libs/include/fmod" PARENT_SCOPE )

if( IOS )
    set( ADDED_CPP_PATTERNS  IOS/*.mm ) 	
    set( ADDED_H_PATTERNS    *.hpp IOS/*.h  )
    set( INCLUDES_DIR  ${INCLUDES_DIR} "${PROJECT_SOURCE_DIR}/Sound/iOS" PARENT_SCOPE )    
endif()

define_source_files (
GLOB_CPP_PATTERNS *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS   *.h   ${ADDED_H_PATTERNS}
PARENT_SCOPE)