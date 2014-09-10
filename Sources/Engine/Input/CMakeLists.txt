if( IOS )
    set( ADDED_CPP_PATTERNS  *.mm ) 	
    set( ADDED_H_PATTERNS    )
    set( ERASE_FILES    GamepadDevice.cpp  )
endif()

define_source_files (
GLOB_CPP_PATTERNS *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS   *.h   ${ADDED_H_PATTERNS}
GLOB_ERASE_FILES  ${ERASE_FILES}
PARENT_SCOPE)

