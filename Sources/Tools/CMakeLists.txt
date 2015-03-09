cmake_minimum_required ( VERSION 2.8 )
                     
project ( TextureTool )

add_definitions       ( 
-D_CRT_SECURE_NO_DEPRECATE
-DSTATIC_MAGICK 
-D_MAGICKLIB_
-D_LIB
-DDDARWIN_NO_CARBON
)

include_directories            ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib ) 
include_directories            ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )  

define_source_folders          ( GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

add_library ( TextureTool STATIC ${PROJECT_SOURCE_FILES} )

if( APPLE )
    target_link_libraries  ( TextureTool libbz2.dylib libz.dylib stdc++.6.dylib )

elseif( WIN32 )
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP" ) 
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP" )

    target_link_libraries  ( TextureTool  ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/IMagickHelper.lib )

endif()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG} )
    target_link_libraries  ( TextureTool debug ${FILE} )
endforeach ()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE} )
    target_link_libraries  ( TextureTool optimized ${FILE} general ${FILE} )
endforeach ()

	