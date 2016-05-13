set( ADDED_CPP_PATTERNS *.cpp Private/*.cpp)
set( ADDED_H_PATTERNS   *.h   Private/*.h)

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

define_source_files( PARENT_SCOPE
  GLOB_CPP_PATTERNS    ${ADDED_CPP_PATTERNS}
  GLOB_H_PATTERNS      ${ADDED_H_PATTERNS}
)
