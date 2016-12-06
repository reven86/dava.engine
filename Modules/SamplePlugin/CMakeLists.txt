cmake_minimum_required (VERSION 3.0)
set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common  )

set( NAME_MODULE SamplePlugin )                  # set name to plugin 

set( MODULE_TYPE PLUGIN )                        # set module type [INLINE, STATIC, PLUGIN]

set( FIND_PACKAGE  DavaFramework )

set( INCLUDES "." )                              # set search path headers for all project
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR} )       # set search path headers inside module

set( HPP_FILES *.h )                             # 
set( CPP_FILES Private/*.cpp )                   # set where are sources

setup_main_module()                              # induce macros of module formation