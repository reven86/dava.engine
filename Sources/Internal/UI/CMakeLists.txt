set( ADDED_CPP_PATTERNS *.cpp Components/*.cpp Layouts/*.cpp  Styles/*.cpp  Private/*.cpp)  
set( ADDED_H_PATTERNS   *.h   Components/*.h   Layouts/*.h    Styles/*.h    Private/*.h)


if(ANDROID)
    list( APPEND ADDED_CPP_PATTERNS Private/Android/*.cpp)
    list( APPEND ADDED_H_PATTERNS Private/Android/*.h)
endif()

if(IOS)
    list( APPEND ADDED_CPP_PATTERNS Private/iOS/*.cpp Private/iOS/*.mm)
    list( APPEND ADDED_H_PATTERNS Private/iOS/*.h)
endif()

if(MACOS)
    list( APPEND ADDED_CPP_PATTERNS Private/MacOS/*.cpp Private/MacOS/*.mm)
    list( APPEND ADDED_H_PATTERNS Private/MacOS/*.h)
endif()

if(WIN32)
    list( APPEND ADDED_CPP_PATTERNS Private/Win32/*.cpp)
    list( APPEND ADDED_H_PATTERNS Private/Win32/*.h)
endif()

if(WINDOWS_UAP)
    list( APPEND ADDED_CPP_PATTERNS Private/UWP/*.cpp)
    list( APPEND ADDED_H_PATTERNS Private/UWP/*.h)
endif()



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
