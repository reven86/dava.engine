
set( NAME_MODULE SampleModule )                  # set name to module 

set( MODULE_TYPE STATIC )                        # set module type [INLINE, STATIC, DYNAMIC]

set( INCLUDES "." )                              # set search path headers for all project
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR} )       # set search path headers inside module

set( MODULE_INITIALIZATION_CODE SampleModule.h ) # set hider where found module
set( MODULE_INITIALIZATION_NAMESPACE Test )      # set module namespace
set( HPP_FILES *.h )                             # 
set( CPP_FILES Private/*.cpp )                   # set where are sources

set( EXCLUDE_FROM_ALL true )                     

setup_main_module()                              # induce macros of module formation