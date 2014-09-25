cmake_minimum_required (VERSION 2.8)
project      ( UIEditor )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../CMake/Modules/" ) 

find_package ( DavaFramework REQUIRED )
find_package ( Qt4           REQUIRED )

include      ( CMake-common )
include      ( ${QT_USE_FILE} )

define_source_folders ( GLOB_FOLDER "Classes" GLOB_ERASE_FOLDERS "_build" )

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${DAVA_IMAGE_MAGICK_INCLUDES_PATH} ) 
include_directories   (.)
include_directories   ( "../TextureTools" )
include_directories   ( "Classes" )
include_directories   ( "Classes/Models" )
include_directories   ( "Classes/Metadata" )
include_directories   ( "Classes/Controllers" )
include_directories   ( "Classes/Commands" )
include_directories   ( "Classes/Helpers" )
include_directories   ( "Classes/UI" )
include_directories   ( "Classes/UI/Dialogs" )
include_directories   ( "Classes/UI/PropertyGridWidgets" )


add_definitions       ( 
-DDAVA_DEBUG
-D_CRT_SECURE_NO_DEPRECATE
-DDAVA_QT
-DQT_HAVE_MMX
-DQT_HAVE_3DNOW
-DQT_HAVE_SSE
-DQT_HAVE_MMXEXT
-DQT_HAVE_SSE2
-DQT_LARGEFILE_SUPPORT
-DQT_THREAD_SUPPORT
)

if (MSVC)
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
    add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo" )
 
    set( LIBRARIES  "gdi32.lib"  "opengl32.lib" "glu32.lib" "winmm.lib" )                            
    set( LIBRARIES_DEBUG ${LIBRARIES}  "msvcrtd.lib"  )    
    set( LIBRARIES       ${LIBRARIES}  "msvcrt.lib"   )     

endif()

file              ( GLOB_RECURSE QT_LIST "Classes/*.h"  )
qt4_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "Ui/*.ui" )
qt4_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "Data/*.qrc" )
qt4_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../TextureTools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

generate_source_groups_project ()
                               
add_executable  ( UIEditor 
    ${MOC_APP_SRCS} 
    ${UI_APP_HDRS} 
    ${RCC_APP_SRCS} 
    ${PROJECT_SOURCE_FILES} 
)

if (MSVC)
    set_target_properties ( UIEditor PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"") #libc.lib;
endif()

target_link_libraries( UIEditor
    ${DAVA_LIBRARY}
    ${QT_LIBRARIES} 
    TextureCompression 
    TexturePacker 
    debug   ${LIBRARIES_DEBUG} 
    general ${LIBRARIES} 
)

