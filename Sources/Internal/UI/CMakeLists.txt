set( HPP_FILES *.h   Components/*.h   Layouts/*.h    Styles/*.h   Focus/*.h   Private/*.h )
set( CPP_FILES *.cpp Components/*.cpp Layouts/*.cpp  Styles/*.cpp Focus/*.cpp Private/*.cpp )
set( CPP_FILES_APPLE *.mm )

set( HPP_FILES_ANDROID Private/android/*.cpp )
set( CPP_FILES_ANDROID Private/android/*.h   )

set( HPP_FILES_IOS Private/ios/*.h )
set( CPP_FILES_IOS Private/ios/*.cpp Private/ios/*.mm  )

set( HPP_FILES_MACOS Private/osx/*.h )
set( CPP_FILES_MACOS Private/osx/*.cpp Private/osx/*.mm )

set( HPP_FILES_WIN Private/win32/*.h )
set( CPP_FILES_WIN Private/win32/*.cpp )

set( HPP_FILES_WINUAP Private/uwp/*.h )
set( CPP_FILES_WINUAP Private/uwp/*.cpp )

set( ERASE_FILES_NOT_ANDROID     UITextFieldAndroid.cpp 
                                 UITextFieldAndroid.h 
                                 UIScreenManagerAndroid.h 
                                 UIScreenManagerAndroid.cpp )

set( ERASE_FILES_NOT_IOS         UIScreenManageriPhone.h 
                                 UIScreenManageriPhoneImpl.h )

set( ERASE_FILES_NOT_MACOS       UIScreenManagerDefault.h  
                                 UIScreenManagerDefault.cpp  )

set( ERASE_FILES_NOT_WIN         ${ERASE_FILES_NOT_MACOS} )
set( ERASE_FILES_NOT_WINUAP      ${ERASE_FILES_NOT_MACOS} )

setup_main_module()