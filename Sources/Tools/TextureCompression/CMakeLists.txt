cmake_minimum_required (VERSION 2.8)
project ( TextureCompression )
                          
define_source_folders          ( GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
    add_definitions( "/LTCG" )
endif()

set         ( TEXTURE_COMPRESSION_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} )
add_library ( TextureCompression STATIC ${PROJECT_SOURCE_FILES} )



