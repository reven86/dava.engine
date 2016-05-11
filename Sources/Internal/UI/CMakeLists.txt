set( ADDED_CPP_PATTERNS *.cpp Components/*.cpp Layouts/*.cpp  Styles/*.cpp Focus/*.cpp Private/*.cpp Input/*.cpp)
set( ADDED_H_PATTERNS   *.h   Components/*.h   Layouts/*.h    Styles/*.h   Focus/*.h   Private/*.h   Input/*.h)

if(ANDROID)
    list( APPEND ADDED_CPP_PATTERNS Private/android/*.cpp)
    list( APPEND ADDED_H_PATTERNS Private/android/*.h)
endif()

if(IOS)
    list( APPEND ADDED_CPP_PATTERNS Private/ios/*.cpp Private/ios/*.mm)
    list( APPEND ADDED_H_PATTERNS Private/ios/*.h)
endif()

if(MACOS)
    list( APPEND ADDED_CPP_PATTERNS Private/osx/*.cpp Private/osx/*.mm)
    list( APPEND ADDED_H_PATTERNS Private/osx/*.h)
endif()

if((WIN32) AND (NOT WINDOWS_UAP))
    list( APPEND ADDED_CPP_PATTERNS Private/win32/*.cpp)
    list( APPEND ADDED_H_PATTERNS Private/win32/*.h)
endif()

if(WINDOWS_UAP)
    list( APPEND ADDED_CPP_PATTERNS Private/uwp/*.cpp)
    list( APPEND ADDED_H_PATTERNS Private/uwp/*.h)
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
