
if(ANDROID)
    set( ERASE_FILES UIScreenManagerMacOS.cpp UIScreenManagerMacOS.h )
endif()

# Define source files
define_source_files ( PARENT_SCOPE GLOB_ERASE_FILES ${ERASE_FILES} )

