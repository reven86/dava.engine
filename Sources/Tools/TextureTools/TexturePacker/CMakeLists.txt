cmake_minimum_required (VERSION 2.8)

project ( TexturePacker )


add_definitions       ( 
-D_CRT_SECURE_NO_DEPRECATE
-DSTATIC_MAGICK 
-D_MAGICKLIB_
-D_LIB
-DDDARWIN_NO_CARBON
)

include_directories            ( ${DAVA_IMAGE_MAGICK_INCLUDES_PATH} ) 
       
define_source_folders          ( GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

set         ( TEXTURE_PACKER_COMPRESSION_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} )
add_library ( TexturePacker STATIC ${PROJECT_SOURCE_FILES} )

if( APPLE )
	target_link_libraries  ( TexturePacker libbz2.dylib libz.dylib stdc++.6.dylib )
endif()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG} )
    target_link_libraries  ( TexturePacker debug ${FILE} )
endforeach ()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE} )
    target_link_libraries  ( TexturePacker optimized ${FILE} general ${FILE} )
endforeach ()

