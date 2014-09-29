cmake_minimum_required (VERSION 2.8)
project ( TextureCompression )
                        
define_source_folders          ( GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

set         ( TEXTURE_COMPRESSION_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} )
add_library ( TextureCompression STATIC ${PROJECT_SOURCE_FILES} )



