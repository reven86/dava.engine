

if(WIN32)
    set( ERASE_FILES UITextFieldAndroid.cpp UITextFieldAndroid.h UIScreenManagerMacOS.cpp UIScreenManagerMacOS.h )
elseif(ANDROID)
    set( ERASE_FILES UIScreenManagerMacOS.cpp UIScreenManagerMacOS.h )
elseif(APPLE)
    set( ADDED_CPP_PATTERNS  *.mm ) 	
    set( ERASE_FILES UITextFieldAndroid.cpp UITextFieldAndroid.h )
endif()

if( IOS )
    set( ADDED_H_PATTERNS    )
    set( ERASE_FILES 
    UITextFieldAndroid.cpp
    UITextFieldAndroid.h 
    UIScreenManagerMacOS.cpp 
    UIScreenManagerMacOS.h 
    )
endif()

define_source_files (
GLOB_CPP_PATTERNS *.cpp  ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS   *.h   ${ADDED_H_PATTERNS}
GLOB_ERASE_FILES  ${ERASE_FILES}
PARENT_SCOPE)


