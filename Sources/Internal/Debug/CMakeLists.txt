
set( HPP_FILES *.h Private/*.h )
set( CPP_FILES *.cpp Private/*.cpp )
set( CPP_FILES_APPLE *.mm Private/*.mm  )
set( ERASE_FILES_MACOS  UIAlertView_Modal.h
                        UIAlertView_Modal.mm 
                        UIDismissionHandlerAlertView.h 
                        UIDismissionHandlerAlertView.mm )


setup_main_module()
