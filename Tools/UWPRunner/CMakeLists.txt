cmake_minimum_required (VERSION 3.0)
project ( UWPRunner )

# Enable MemoryManagerTest in UnitTests
# this variable should be defined in all dependant projects (dava framework, etc)
if( NOT DAVA_MEGASOLUTION )
    set ( DAVA_MEMORY_PROFILER 1 )
endif()

if ( "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64" )
    set( WARNINGS_AS_ERRORS false )
else()
    set( WARNINGS_AS_ERRORS true )
endif()

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include      ( CMake-common )

find_package( DavaFramework REQUIRED )
find_package( DavaTools     REQUIRED )

if( WIN32 )
    set( PLATFORM_SPECIFIC_FOLDER "Win32Specific" )
    set( EXECUTABLE_FLAG WIN32 )

else ()
    message( FATAL_ERROR "This util can't be built on non-Windows platform" )

endif()

include_directories   ( "Classes" )
define_source_folders ( SRC_ROOT "Classes" ${PLATFORM_SPECIFIC_FOLDER}  )
generate_source_groups_project( SRC_ROOT "Classes" )

setup_main_executable()