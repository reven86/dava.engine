
if( MACOS )
    set( ERASE_FILES  UIAlertView_Modal.h
                      UIAlertView_Modal.m 
                      UIDismissionHandlerAlertView.h 
                      UIDismissionHandlerAlertView.mm )
endif()

define_source_files (PARENT_SCOPE GLOB_ERASE_FILES ${ERASE_FILES} )
