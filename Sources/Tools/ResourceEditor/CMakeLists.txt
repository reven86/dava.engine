cmake_minimum_required (VERSION 2.8)
project ( ResourceEditor )

set          ( CMakeHelloWorld_VERSION_MAJOR 1 )
set          ( CMakeHelloWorld_VERSION_MINOR 0 )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../CMake/Modules/" ) 

find_package ( DavaFramework REQUIRED )
find_package ( Qt4           REQUIRED )

include      ( CMake-common )
include      ( ${QT_USE_FILE} )

define_source_folders ( GLOB_FOLDER "Classes" GLOB_ERASE_FOLDERS "_build" "VersionInfoWidget" )

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${DAVA_IMAGE_MAGICK_INCLUDES_PATH} ) 
include_directories   ( "${DAVA_ROOT_DIR}/dava.speedtree" )
include_directories   ( "${DAVA_ROOT_DIR}/../dava.speedtree/Sources" )
include_directories   ( "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast/wrapper" )
include_directories   ( "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast/beast/include" )
include_directories   ( "${DAVA_THIRD_PARTY_INCLUDES_PATH}/FCollada" )
include_directories   (.)
include_directories   ( "../" )
include_directories   ( "Classes" )
include_directories   ( "Classes/bullet" )
include_directories   ( "Classes/Qt" )
include_directories   ( "Classes/Collada" )


add_definitions       ( 
-D__DAVAENGINE_SPEEDTREE__ 
-DDAVA_DEBUG
-DQT_OPENGL_LIB
-DQT_GUI_LIB
-DQT_NETWORK_LIB
-DQT_CORE_LIB
-DQT_HAVE_MMX
-DQT_HAVE_3DNOW
-DQT_HAVE_SSE
-DQT_HAVE_MMXEXT
-DQT_HAVE_SSE2
-DQT_THREAD_SUPPORT
-D_CRT_SECURE_NO_DEPRECATE
-D__DAVAENGINE_RESOURCEEDITOR__
-DDAVA_QT
-D__DAVAENGINE_BEAST__
-DDAVA_FMOD
)

if (MSVC)
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
    add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo" )
    set( DAVA_LIBRARY ${DAVA_LIBRARY}  "msvcmrt.lib" "msvcrt.lib" "gdi32.lib" "opengl32.lib" "glu32.lib" "winmm.lib" )
endif()

set ( PROJECT_SOURCE_FILES ${PROJECT_SOURCE_FILES} "${DAVA_ROOT_DIR}/../dava.speedtree/Sources/SpeedTreeImporter.h" 
                                                   "${DAVA_ROOT_DIR}/../dava.speedtree/Sources/SpeedTreeImporter.cpp" )

file              ( GLOB_RECURSE QT_LIST "Classes/Qt/*.h"  )
qt4_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt4_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt4_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../TextureCompression" ${CMAKE_CURRENT_BINARY_DIR}/TextureCompression )
add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../TexturePacker" ${CMAKE_CURRENT_BINARY_DIR}/TexturePacker )

generate_source_groups_project ()

file ( GLOB_RECURSE BEAST_LIST "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast/wrapper/*.cpp"  
                               "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast/wrapper/*.h"  )
                               
add_executable  ( ResourceEditor 
    ${MOC_APP_SRCS} 
    ${UI_APP_HDRS} 
    ${RCC_APP_SRCS} 
    ${PROJECT_SOURCE_FILES_HPP} 
    ${PROJECT_SOURCE_FILES_CPP}  
    "${DAVA_ROOT_DIR}/../dava.speedtree/Sources/SpeedTreeImporter.cpp"
    ${BEAST_LIST}
)

if (MSVC)
    set_target_properties ( ResourceEditor PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"") #libc.lib;
endif()

target_link_libraries ( ResourceEditor
    ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH} 
    ${QT_LIBRARIES} 
    ${DAVA_LIBRARY} 
    TextureCompression 
    TexturePacker 
    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/FCollada.lib" 
    "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast/beast/lib/beast32.lib"
  )

