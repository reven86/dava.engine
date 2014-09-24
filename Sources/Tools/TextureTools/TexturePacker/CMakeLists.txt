cmake_minimum_required (VERSION 2.8)

project ( TexturePacker )

include_directories   ( ${DAVA_IMAGE_MAGICK_INCLUDES_PATH} ) 
                          
define_source_folders          ( GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
    add_definitions( "/LTCG" )
endif()

set         ( TEXTURE_PACKER_COMPRESSION_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} )
add_library ( TexturePacker STATIC ${PROJECT_SOURCE_FILES} )

target_link_libraries ( TexturePacker ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH} )



