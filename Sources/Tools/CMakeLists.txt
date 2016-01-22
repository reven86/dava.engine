cmake_minimum_required (VERSION 3.0)
project( DavaTools )

if (WIN32)
    add_definitions (-D_CRT_SECURE_NO_DEPRECATE)
elseif (MACOS AND NOT ANDROID)
    add_definitions (-DDARWIN_NO_CARBON)
endif()

# Igor DO WE NEED next defines?
add_definitions       ( 
-DSTATIC_MAGICK 
-D_MAGICKLIB_
-D_LIB
)

include ( PlatformSettings )

include_directories   ( . )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )  
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib ) 

if( IOS OR ANDROID )
    set( ADDED_CPP_PATTERNS TeamcityOutput/*.cpp  CommandLine/CommandLineParser.cpp )
    set( ADDED_H_PATTERNS   TeamcityOutput/*.h   CommandLine/CommandLineParser.h    )

else()
    set( ADDED_CPP_PATTERNS AssetCache/*/*.cpp AssetCache/*.cpp CommandLine/*.cpp TeamcityOutput/*.cpp TextureCompression/*.cpp TexturePacker/*.cpp )
    set( ADDED_H_PATTERNS   AssetCache/*/*.h   AssetCache/*.h   CommandLine/*.h   TeamcityOutput/*.h   TextureCompression/*.h   TexturePacker/*.h   )

endif()

if( DAVA_USE_QT_WIDGETS )
    add_subdirectory ( "${CMAKE_CURRENT_LIST_DIR}/QtTools" ${CMAKE_CURRENT_BINARY_DIR}/QtTools )
    set( ADDED_LIBS QtTools )
endif()

define_source_files (
GLOB_CPP_PATTERNS  ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    ${ADDED_H_PATTERNS}
)  

set_project_files_properties( "${CPP_FILES}" )

generate_source_groups_project ( )
generated_unity_sources     ( CPP_FILES IGNORE_LIST_APPLE "TeamcityOutput/TeamcityOutput.cpp" )

add_library                 ( ${PROJECT_NAME} STATIC ${CPP_FILES} ${H_FILES} )

if( MACOS AND NOT ANDROID )
    target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} libbz2.dylib libz.dylib stdc++.6.dylib )
elseif( WIN32 )
	if ( X64_MODE )
    	target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/win64/IMagickHelper.lib )
	else ()
    	target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/win32/IMagickHelper.lib )
	endif ()
endif()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} general ${FILE} )
endforeach ()

	