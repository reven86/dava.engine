
if( WIN32 )
    set( ADDED_CPP_PATTERNS  Win32/*.cpp ) 	
    set( ADDED_H_PATTERNS    Win32/*.h  )

else()
    set( ADDED_CPP_PATTERNS  Unix/*.cpp ) 	
    set( ADDED_H_PATTERNS    Unix/*.h  )

endif()


define_source_files (
GLOB_CPP_PATTERNS *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS   *.h   ${ADDED_H_PATTERNS}
PARENT_SCOPE)