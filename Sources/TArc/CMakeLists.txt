cmake_minimum_required (VERSION 3.0)
project( TArc )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../CMake/Modules/" )

if (WIN32)
    add_definitions (-D_CRT_SECURE_NO_DEPRECATE)
elseif (MACOS AND NOT ANDROID)
    add_definitions (-DDARWIN_NO_CARBON)
endif()

include ( CMake-common )

find_package( QT5 REQUIRED)
find_package( DavaFramework REQUIRED )
find_package( DavaTools  REQUIRED )

include_directories   ( . )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )  

define_source()

set_project_files_properties( "${PROJECT_SOURCE_FILES}" )
generated_unity_sources        ( PROJECT_SOURCE_FILES )

add_library                 ( ${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILE_ONLY} )

if( MACOS )
    target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} libbz2.dylib libz.dylib stdc++.6.dylib )
elseif( WIN32 )
   	target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} )
endif()

	
