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

set( ADDED_CPP_PATTERNS  AssetCache/*.cpp TextureCompression/*.cpp TexturePacker/*.cpp ResourceArchiver/*.cpp )
set( ADDED_H_PATTERNS    AssetCache/*.h   TextureCompression/*.h   TexturePacker/*.h   ResourceArchiver/*.h  )

define_source  (  SOURCE ${ADDED_CPP_PATTERNS} ${ADDED_H_PATTERNS} )

set_project_files_properties( "${PROJECT_SOURCE_FILES}" )
generated_unity_sources        ( PROJECT_SOURCE_FILES )

if( COVERAGE AND TARGET_FOLDERS_${PROJECT_NAME} )#tmp
    string(REPLACE ";" " " TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )
    string(REPLACE "\"" "" TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )
    set ( DEFINITIONS -DTARGET_FOLDERS_${PROJECT_NAME}="${TARGET_FOLDERS_${PROJECT_NAME}}" )
    save_property( PROPERTY_LIST  DEFINITIONS )
endif()

add_library                 ( ${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILE_ONLY} )

if( MACOS AND NOT ANDROID )
    target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} libbz2.dylib libz.dylib stdc++.6.dylib )
elseif( WIN32 )
   	target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} )
endif()
