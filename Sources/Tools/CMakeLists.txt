cmake_minimum_required( VERSION 3.0 )

if ( TEXTURE_TOOL_FOUND )
    return ()
endif ()
set ( TEXTURE_TOOL_FOUND 1 )
                     
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
    if( QT5_FOUND )
        include_directories( ${Qt5Core_INCLUDE_DIRS} )     
        include_directories( ${Qt5Gui_INCLUDE_DIRS} )
        include_directories( ${Qt5Widgets_INCLUDE_DIRS} )     
        include_directories( "Widgets" )                                      
    
        set( MOC_APP_SRCS )
        set( UI_APP_HDRS  )
        set( RCC_APP_SRCS )

        file              ( GLOB_RECURSE QT_LIST "Widgets/*.h"  )
        qt5_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

        file              ( GLOB_RECURSE UI_LIST "Widgets/*.ui" )
        qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

        file              ( GLOB_RECURSE RCC_LIST "Widgets/*.qrc" )
        qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

        file( GLOB_RECURSE WIDGETS_SRC "Widgets/*.h"  "Widgets/*.cpp" )
        set( ADDED_SRC ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} ${WIDGETS_SRC} )
        set( Generated ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )

    else()
        message( "Warning: Qt is not found. Can not use QT_WIDGETS" )
     
    endif()

endif()

define_source_files (
GLOB_CPP_PATTERNS  ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS    ${ADDED_H_PATTERNS}
)  

generate_source_groups_project ( GROUP_FOLDERS Generated )
add_library                    ( ${PROJECT_NAME} STATIC ${CPP_FILES} ${H_FILES} ${ADDED_SRC} )

set_dava_target_properties ( ${PROJECT_NAME} )

if( MACOS )
    target_link_libraries  ( ${PROJECT_NAME} libbz2.dylib libz.dylib stdc++.6.dylib )

elseif( WIN32 )
    target_link_libraries  ( ${PROJECT_NAME}  ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/IMagickHelper.lib )

endif()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} general ${FILE} )
endforeach ()

	