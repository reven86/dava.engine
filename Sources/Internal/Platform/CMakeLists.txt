set( HPP_FILES *.h   )
set( CPP_FILES *.cpp )
set( CPP_FILES_APPLE *.mm  )

set( CPP_FILES_IOS TemplateiOS/*.cpp TemplateiOS/*.mm  )
set( HPP_FILES_IOS TemplateiOS/*.h  )

set( CPP_FILES_MACOS TemplateMacOS/*.cpp TemplateMacOS/*.mm  )
set( HPP_FILES_MACOS TemplateMacOS/*.h  )

set( CPP_FILES_WIN   TemplateWin32/*.cpp )
set( HPP_FILES_WIN   TemplateWin32/*.h  )

set( CPP_FILES_WINUAP TemplateWin32/*.cpp )
set( HPP_FILES_WINUAP TemplateWin32/*.h  )

set( CPP_FILES_UNIX Linux/*.cpp )
set( HPP_FILES_UNIX Linux/*.h  )

set( ERASE_FILES_ANDROID  )

set( ERASE_FILES_IOS  )

set( ERASE_FILES_MACOS AppDelegate.h AppDelegate.mm HelperAppDelegate.h HelperAppDelegate.mm MainWindowController.h MainWindowController.mm OpenGLView.h OpenGLView.mm CoreMacOSPlatform.h )

set( ERASE_FILES_WIN CorePlatformWin32.cpp CorePlatformWin32.h )
set( ERASE_FILES_WINUAP ${ERASE_FILES_WIN32} )

setup_main_module()
