
set( NAME_MODULE SampleModule )

set( MODULE_TYPE STATIC )#INLINE STATIC DYNAMIC 

set( INCLUDES "." )
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR} )

set( MODULE_INITIALIZATION_CODE SampleModule.h )
set( HPP_FILES *.h )
set( CPP_FILES Private/*.cpp )

setup_main_module()
