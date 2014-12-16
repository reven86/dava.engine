
set( INCLUDES_DIR  ${INCLUDES_DIR} "${PROJECT_SOURCE_DIR}/../../Libs/include/fmod" PARENT_SCOPE )

if( DISABLE_SOUNDS )

	define_source_files ( 
	EXTRA_H_FILES   ${CMAKE_CURRENT_LIST_DIR}/SoundEvent.h ${CMAKE_CURRENT_LIST_DIR}/SoundSystem.h  
	EXTRA_CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/SoundSystemEmpty.cpp
	GLOB_CPP_PATTERNS null  
	GLOB_H_PATTERNS   null
	PARENT_SCOPE )
	
else()

	if( IOS )
	    set( ADDED_CPP_PATTERNS  IOS/*.mm ) 	
	    set( ADDED_H_PATTERNS    *.hpp IOS/*.h  )
	    set( ADD_ENGINE_INCLUDES_DIR  ${ADD_ENGINE_INCLUDES_DIR} "${PROJECT_SOURCE_DIR}/Sound/iOS" PARENT_SCOPE )    
	endif()

	define_source_files (
	GLOB_CPP_PATTERNS *.cpp ${ADDED_CPP_PATTERNS}
	GLOB_H_PATTERNS   *.h   ${ADDED_H_PATTERNS}
        GLOB_ERASE_FILES  SoundSystemEmpty.cpp
	PARENT_SCOPE )

endif()
