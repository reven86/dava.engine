set( HPP_FILES *.h Private/*.h)
set( CPP_FILES *.cpp Private/*.cpp)
set( CPP_FILES_APPLE *.mm Private/*.mm)
set( ERASE_FILES_MACOS  Private/UIAlertView_Modal.h
                        Private/UIAlertView_Modal.mm 
                        Private/UIDismissionHandlerAlertView.h 
                        Private/UIDismissionHandlerAlertView.mm)

setup_main_module()
