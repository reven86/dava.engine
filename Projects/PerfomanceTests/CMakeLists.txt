cmake_minimum_required (VERSION 2.8)

project      ( PerfomanceTestsLib )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

include      ( CMake-common )
include      ( FindDavaFramework )

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( "Classes" )
include_directories   ( "Classes/Infrastructure" )

set( TeamcityOutput          ${DAVA_TOOLS_DIR}/TeamcityOutput  )
set( PackerTools             ${DAVA_TOOLS_DIR}/TexturePacker )
set( PackerToolsSCR          ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.h 
                             ${DAVA_TOOLS_DIR}/TexturePacker/CommandLineParser.cpp )

if( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
endif()

define_source_folders( SRC_ROOT "Classes"  ${PLATFORM_SPECIFIC_FOLDER} ${TeamcityOutput} )
generate_source_groups_project( )


set( ADDED_SRC                   ${PackerToolsSCR} )
set( LIBRARIES                   )
set( LIBRARIES_RELEASE           )
set( LIBRARIES_DEBUG             )
set( ADDED_BINARY_DIR            )
set( EXECUTABLE_FLAG             WIN32)

setup_main_executable()