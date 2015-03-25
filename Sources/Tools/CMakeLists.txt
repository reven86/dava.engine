cmake_minimum_required( VERSION 3.0 )
                   
project( DavaTools )

add_definitions       ( 
-D_CRT_SECURE_NO_DEPRECATE
-DSTATIC_MAGICK 
-D_MAGICKLIB_
-D_LIB
-DDDARWIN_NO_CARBON
)

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )  
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib ) 


if( IOS OR ANDROID )
    set( ADDED_CPP_PATTERNS TeamcityOutput/*.cpp  TexturePacker/CommandLineParser.cpp )
    set( ADDED_H_PATTERNS   TeamcityOutput/*.h   TexturePacker/CommandLineParser.h    )

else()
    set( ADDED_CPP_PATTERNS TeamcityOutput/*.cpp TextureCompression/*.cpp TexturePacker/*.cpp )
    set( ADDED_H_PATTERNS   TeamcityOutput/*.h   TextureCompression/*.h   TexturePacker/*.h   )

endif()

if( IOS )
    set_source_files_properties( ${CMAKE_CURRENT_LIST_DIR}/TeamcityOutput/TeamcityOutput.cpp 
                                 PROPERTIES COMPILE_FLAGS "-x objective-c++" )  
endif()

if( DAVA_USE_QT_WIDGETS )
    add_subdirectory ( "${CMAKE_CURRENT_LIST_DIR}/QtTools" ${CMAKE_CURRENT_BINARY_DIR}/QtTools )
    set( ADDED_LIBS QtTools )
endif()

define_source_files (
GLOB_CPP_PATTERNS  ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    ${ADDED_H_PATTERNS}
)  

generate_source_groups_project ( )
add_library                    ( ${PROJECT_NAME} STATIC ${CPP_FILES} ${H_FILES} )

if( MACOS )
    target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} libbz2.dylib libz.dylib stdc++.6.dylib )

elseif( WIN32 )
    target_link_libraries  ( ${PROJECT_NAME} ${ADDED_LIBS} ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/IMagickHelper.lib )

endif()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} general ${FILE} )
endforeach ()

	