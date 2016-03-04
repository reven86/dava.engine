set( ADDED_CPP_PATTERNS *.cpp Components/*.cpp Layouts/*.cpp  Styles/*.cpp Focus/*.cpp Actions/*.cpp)  
set( ADDED_H_PATTERNS   *.h   Components/*.h   Layouts/*.h    Styles/*.h   Focus/*.h   Actions/*.h)

if( NOT ANDROID )
    list( APPEND ERASE_FILES UITextFieldAndroid.cpp 
                             UITextFieldAndroid.h
                             UIScreenManagerAndroid.h 
                             UIScreenManagerAndroid.cpp )
endif ()

if ( ( NOT MACOS ) AND ( NOT WIN32 ) ) 
    list( APPEND ERASE_FILES UIScreenManagerDefault.h 
                             UIScreenManagerDefault.cpp )
endif ()

if( NOT IOS )
    list( APPEND ERASE_FILES UIScreenManageriPhone.h 
                             UIScreenManageriPhoneImpl.h )    
endif()

if( APPLE )
    list( APPEND ADDED_CPP_PATTERNS *.mm )    
endif()

define_source_files( PARENT_SCOPE 
  GLOB_CPP_PATTERNS    ${ADDED_CPP_PATTERNS}
  GLOB_H_PATTERNS      ${ADDED_H_PATTERNS}
  GLOB_ERASE_FILES     ${ERASE_FILES} 
)

