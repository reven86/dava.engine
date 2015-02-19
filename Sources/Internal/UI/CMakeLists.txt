set( ADDED_CPP_PATTERNS *.cpp Components/*.cpp   )  
set( ADDED_H_PATTERNS   *.h   Components/*.h     )

if( WIN32 )
    set( ERASE_FILES 	UITextFieldAndroid.cpp 
    					UITextFieldAndroid.h
    					UIScreenManagerAndroid.h 
    					UIScreenManagerAndroid.cpp )

elseif( ANDROID )
    set( ERASE_FILES 	UIScreenManagerMacOS.cpp 
    					UIScreenManagerMacOS.h )

elseif( MACOS )
    set( ERASE_FILES 	UITextFieldAndroid.cpp 
    					UITextFieldAndroid.h )
    list( APPEND ADDED_CPP_PATTERNS *.mm )    

elseif( IOS )
    set( ERASE_FILES 	UITextFieldAndroid.cpp 
    					UITextFieldAndroid.h 
    					UIScreenManagerMacOS.cpp 
    					UIScreenManagerMacOS.h 
    					UIScreenManagerAndroid.h 
    					UIScreenManagerAndroid.cpp )
    list( APPEND ADDED_CPP_PATTERNS *.mm )    
    
endif()

define_source_files( PARENT_SCOPE 
  GLOB_CPP_PATTERNS    ${ADDED_CPP_PATTERNS}
  GLOB_H_PATTERNS      ${ADDED_H_PATTERNS}
  GLOB_ERASE_FILES     ${ERASE_FILES} 
)

