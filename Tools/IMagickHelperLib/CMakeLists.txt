cmake_minimum_required ( VERSION 3.0 )

project      ( IMagickHelper )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 


include      ( CMake-common         )
include      ( GenerateExportHeader )

set_property ( GLOBAL PROPERTY USE_FOLDERS ON )

add_definitions ( 
    -DSTATIC_MAGICK 
    -D_MAGICKLIB_   
    -D_LIB 
    -D_CONSOLE
    -D__SECURE_SCL 
    -D_SCL_SECURE_NO_WARNING
    -D_CRT_SECURE_NO_DEPRECATE 
    -D_CRT_NON_CONFORMING_SWPRINTFS
    -D_ITERATOR_DEBUG_LEVEL=0
)


set ( LIBRARIES_DIR  "${CMAKE_CURRENT_LIST_DIR}" )

if( APPLE ) 
    
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

    set( IMAGE_MAGICK ImageMagick_mac )

    set ( IMAGE_MAGICK_INCLUDES_PATH  
         "${LIBRARIES_DIR}/${IMAGE_MAGICK}/include/ImageMagick"
         "${LIBRARIES_DIR}/${IMAGE_MAGICK}/" 
         )

    set ( IMAGE_MAGICK_LIBRARIES_PATH  
         "${LIBRARIES_DIR}/${IMAGE_MAGICK}" )  

    file ( GLOB IMAGE_MAGICK_LIBRARIES_RELEASE  ${IMAGE_MAGICK_LIBRARIES_PATH}/lib/*.a)
    file ( GLOB IMAGE_MAGICK_LIBRARIES_DEBUG    ${IMAGE_MAGICK_LIBRARIES_PATH}/lib/*.a )

    list ( APPEND LIBRARIES libbz2.dylib ) 

    find_package(ZLIB )
    list (APPEND LIBRARIES          ${ZLIB_LIBRARY}     )

    find_package(PNG)
    list (APPEND LIBRARIES          ${PNG_LIBRARY}     )

elseif( WIN32 )
    set( IMAGE_MAGICK ImageMagick_win )

    set ( IMAGE_MAGICK_INCLUDES_PATH  
    "${LIBRARIES_DIR}/${IMAGE_MAGICK}/include" )

    set ( IMAGE_MAGICK_LIBRARIES_PATH  
    "${LIBRARIES_DIR}/${IMAGE_MAGICK}/lib" )  

    file ( GLOB IMAGE_MAGICK_LIBRARIES_RELEASE  "${IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )
    file ( GLOB IMAGE_MAGICK_LIBRARIES_DEBUG    "${IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )

endif  ()

include_directories (.)
include_directories ( ${IMAGE_MAGICK_INCLUDES_PATH} ) 
include_directories ( ${DAVA_INCLUDE_DIR} ) 

file ( GLOB LIB_SOURCES "*.h" "*.cpp" )
generate_source_groups_project ()

add_library( ${PROJECT_NAME} SHARED  ${LIB_SOURCES} )

#generate_export_header( ${PROJECT_NAME} )
#file( COPY  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h DESTINATION  ${CMAKE_BINARY_DIR} )
 
set_property( TARGET ${PROJECT_NAME} PROPERTY FOLDER "libraries")

if( APPLE )

    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
    install_name_tool -id  ./lib${PROJECT_NAME}.dylib #@executable_path/../Frameworks
                           ${CMAKE_BINARY_DIR}/$<CONFIG>/lib${PROJECT_NAME}.dylib
    )

endif()

target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} )

foreach ( FILE ${IMAGE_MAGICK_LIBRARIES_DEBUG} ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${IMAGE_MAGICK_LIBRARIES_RELEASE} ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()


