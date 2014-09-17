cmake_minimum_required (VERSION 2.8)
project ( ResourceEditor )

set (CMakeHelloWorld_VERSION_MAJOR 1)
set (CMakeHelloWorld_VERSION_MINOR 0)

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../CMake/Modules/" ) 
include      ( CMake-common )

find_package ( DavaFramework REQUIRED )

define_source_folders  ( GLOB_FOLDER "Classes" GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

include_directories   ( ${DAVA_INCLUDE_DIR} ) 

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
endif()

add_executable  ( ResourceEditor  ${PROJECT_SOURCE_FILES} )

set_target_properties (ResourceEditor PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib\"") 
target_link_libraries ( ResourceEditor  DavaFramework ${DAVA_THIRD_PARTY_LIB} )

