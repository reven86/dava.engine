set( MODULE_TYPE INLINE )

set( HPP_FILES *.h   Private/*.h )
set( CPP_FILES *.cpp Private/*.cpp )

set( HPP_FILES_ANDROID Private/Android/*.h )
set( CPP_FILES_ANDROID Private/Android/*.cpp )

set( HPP_FILES_IOS *.h  Private/Ios/*.h )
set( CPP_FILES_IOS *.mm Private/Ios/*.cpp Private/Ios/*.mm )

set( HPP_FILES_MACOS Private/Mac/*.h )
set( CPP_FILES_MACOS Private/Mac/*.cpp Private/Mac/*.mm )

set( HPP_FILES_WIN Private/Win32/*.h )
set( CPP_FILES_WIN Private/Win32/*.cpp )

set( HPP_FILES_WINUAP Private/Win10/*.h )
set( CPP_FILES_WINUAP Private/Win10/*.cpp )

setup_main_module()
