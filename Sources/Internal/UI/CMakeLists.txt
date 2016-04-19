
set( HPP_FILES_RECURSE           *.h   )
set( CPP_FILES_RECURSE           *.cpp )
set( CPP_FILES_RECURSE_APPLE     *.mm  )

set( ERASE_FILES_NOT_ANDROID     UITextFieldAndroid.cpp 
                                 UITextFieldAndroid.h 
                                 UIScreenManagerAndroid.h 
                                 UIScreenManagerAndroid.cpp )

set( ERASE_FILES_NOT_IOS         UIScreenManageriPhone.h 
                                 UIScreenManageriPhoneImpl.h )

set( ERASE_FILES_NOT_MACOS       UIScreenManagerDefault.h  
                                 UIScreenManagerDefault.cpp  )

set( ERASE_FILES_NOT_WIN32       ${ERASE_FILES_NOT_MACOS} )
set( ERASE_FILES_NOT_WINUAP      ${ERASE_FILES_NOT_MACOS} )

setup_main_module()
