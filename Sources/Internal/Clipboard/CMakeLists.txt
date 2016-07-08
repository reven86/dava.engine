set( HPP_FILES *.h   Private/*.h )
set( CPP_FILES *.cpp Private/*.cpp )
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

setup_main_module()