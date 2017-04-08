cmake_minimum_required (VERSION 3.0)

set( NAME_MODULE EmbeddedWebServer )                    # set name to plugin

set( MODULE_TYPE STATIC )                               # set module type [INLINE, STATIC, PLUGIN]

set( INCLUDES "." )                                     # set search path headers for all project

set( HPP_FILES EmbeddedWebServer.h
               Private/mongoose.h )                     #
set( CPP_FILES Private/EmbeddedWebServer.cpp            #
               )                                        # set where are sources

setup_main_module()                                     # induce macros of module formation
