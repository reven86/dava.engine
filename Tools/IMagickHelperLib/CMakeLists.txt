cmake_minimum_required (VERSION 3.0)
project      ( IMagickHelper )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 


include      ( CMake-common         )
include      ( GenerateExportHeader )

set_property ( GLOBAL PROPERTY USE_FOLDERS ON )

add_definitions ( 
    -D_LIB 
    -D_CONSOLE
    -D__SECURE_SCL 
    -D_SCL_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_DEPRECATE 
    -D_CRT_NON_CONFORMING_SWPRINTFS
)

set ( LIBRARIES_DIR  "${CMAKE_CURRENT_LIST_DIR}/lib" )

list (APPEND LIBRARIES "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac/libpsd.a" )

if( APPLE )
    find_package(ZLIB )
    list (APPEND LIBRARIES ${ZLIB_LIBRARY} )
elseif( WIN32 )

endif  ()

include_directories (.)
include_directories ( ${DAVA_INCLUDE_DIR} )
include_directories ( ${DAVA_THIRD_PARTY_ROOT_PATH}/libpsd/include )
include_directories ( ${DAVA_THIRD_PARTY_ROOT_PATH}/libpng/include/libpng )

file ( GLOB LIB_SOURCES "*.h" "*.cpp" )
file ( GLOB LIB_SOURCES_PNG "${DAVA_THIRD_PARTY_ROOT_PATH}/libpng/src/*.c" )

generate_source_groups_project ()

add_library( ${PROJECT_NAME} SHARED  ${LIB_SOURCES} ${LIB_SOURCES_PNG} )

set_property( TARGET ${PROJECT_NAME} PROPERTY FOLDER "libraries")

if( APPLE )

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND install_name_tool -id "@rpath/libIMagickHelper.dylib" ${CMAKE_BINARY_DIR}/$<CONFIG>/lib${PROJECT_NAME}.dylib
        COMMENT "Run install_name_tool")

endif()

target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} )
