cmake_minimum_required (VERSION 2.8)

project ( TexturePacker )

include_directories   ( ${DAVA_IMAGE_MAGICK_INCLUDES_PATH} ) 
                          
define_source_folders          ( GLOB_ERASE_FOLDERS "_build"  )
generate_source_groups_project ()

set         ( TEXTURE_PACKER_COMPRESSION_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} )
add_library ( TexturePacker STATIC ${PROJECT_SOURCE_FILES} )

file ( GLOB LIBRARIES_RELEASE  "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )
file ( GLOB LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_DB_*.lib" )

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( TexturePacker debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( TexturePacker optimized ${FILE} )
endforeach ()




