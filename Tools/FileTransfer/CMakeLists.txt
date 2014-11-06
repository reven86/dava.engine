cmake_minimum_required (VERSION 2.8)
project ( FileTransfer )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

set          ( DISABLE_SOUNDS 1 )
find_package ( DavaFramework REQUIRED )

include      ( CMake-common )

define_source_folders ( GLOB_ERASE_FOLDERS "_build" "msvc" )
generate_source_groups_project ()

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )


include_directories   (.)


add_definitions       ( 
-D__DAVAENGINE_SPEEDTREE__ 
-D_CRT_SECURE_NO_DEPRECATE
-D__DAVAENGINE_RESOURCEEDITOR__
-DDAVA_DEBUG
-D_UNICODE 
-DUNICODE
)

if( MSVC )    
    set( LIBRARIES "gdi32.lib" "glu32.lib" "psapi.lib" )
    set( LIBRARIES_DEBUG    ${CMAKE_CURRENT_LIST_DIR}/libuv/lib/libuvd.lib  )   
    set( LIBRARIES_RELEASE  ${CMAKE_CURRENT_LIST_DIR}/libuv/lib/libuv.lib   ) 

endif()

if( MACOS )
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")
        
    set( LIBRARIES_DEBUG    ${CMAKE_CURRENT_LIST_DIR}/libuv/lib/libuv_macos.a  )   
    set( LIBRARIES_RELEASE  ${CMAKE_CURRENT_LIST_DIR}/libuv/lib/libuv_macos.a  ) 
endif()
                              
add_executable( ${PROJECT_NAME} ${PROJECT_SOURCE_FILES} ${RESOURCES_LIST} )

if( APPLE )
    
elseif( MSVC )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"")
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/editor.in
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )
endif()

target_link_libraries ( ${PROJECT_NAME}
    ${DAVA_LIBRARY}
    ${LIBRARIES} 
)
 
foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()                                        

