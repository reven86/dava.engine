
if( MACOS )
    set( ERASE_FILES    GamepadDevice.mm 
    					GamepadManager.h 
    					GamepadManager.mm )

elseif( IOS )
    set( ERASE_FILES  	GamepadDevice.cpp )

endif()

define_source_files (PARENT_SCOPE GLOB_ERASE_FILES ${ERASE_FILES} )
