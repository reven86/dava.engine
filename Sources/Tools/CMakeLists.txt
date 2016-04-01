cmake_minimum_required (VERSION 3.0)
project( DavaTools )

if (WIN32)
    add_definitions (-D_CRT_SECURE_NO_DEPRECATE)
elseif (MACOS AND NOT ANDROID)
    add_definitions (-DDARWIN_NO_CARBON)
endif()

include ( CMake-common )

include_directories   ( . )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )  

set( ADDED_CPP_PATTERNS AssetCache/*/*.cpp AssetCache/*.cpp TextureCompression/*.cpp TexturePacker/*.cpp )
set( ADDED_H_PATTERNS   AssetCache/*/*.h   AssetCache/*.h   TextureCompression/*.h   TexturePacker/*.h   )

if( DAVA_USE_QT_WIDGETS )
    add_subdirectory ( "${CMAKE_CURRENT_LIST_DIR}/QtTools" ${CMAKE_CURRENT_BINARY_DIR}/QtTools )
    set( ADDED_LIBS QtTools )
endif()

define_source_files (
GLOB_CPP_PATTERNS  ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    ${ADDED_H_PATTERNS}
)  

set_project_files_properties( "${CPP_FILES}" )
generate_source_groups_project ()

add_library                 ( ${PROJECT_NAME} STATIC ${CPP_FILES} ${H_FILES} )

if( MACOS AND NOT ANDROID )
    target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} libbz2.dylib libz.dylib stdc++.6.dylib )
elseif( WIN32 )
   	target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} )
endif()

	