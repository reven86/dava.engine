cmake_minimum_required (VERSION 2.8)
project      ( UIEditor )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../CMake/Modules/" ) 

#LINK_LIBRARIES( FrameworkMain )

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

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDAVA_DEBUG" )

if (MSVC)
    set ( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP"  )
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP /fp:fast /Zi /GS- /D _SECURE_SCL=0" )

    set( LIBRARIES         "opengl32.lib" "winmm.lib" )                            
    set( LIBRARIES_DEBUG   "msvcrtd.lib"  )    
    set( LIBRARIES_RELEASE "msvcrt.lib"   )     

elseif( APPLE )

    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

    file ( GLOB_RECURSE RESOURCES_LIST "${CMAKE_CURRENT_LIST_DIR}/Data/*" )
    foreach ( FILE ${RESOURCES_LIST} )

        get_filename_component ( FILE_PATH ${FILE} PATH ) 
        STRING(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" FILE_GROUP ${FILE_PATH} )

        set_source_files_properties( ${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${FILE_GROUP} )
    endforeach ()


    file ( GLOB DYLIB_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)

    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks )


    list ( APPEND RESOURCES_LIST  ${DYLIB_FILES} ) 
    list ( APPEND LIBRARIES       ${DYLIB_FILES} )

endif()

file              ( GLOB_RECURSE QT_LIST "Classes/*.h" )
qt4_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "Ui/*.ui" )
qt4_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "Data/*.qrc" )
qt4_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../TextureTools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

generate_source_groups_project ()
                               
add_executable  ( UIEditor MACOSX_BUNDLE
    ${MOC_APP_SRCS} 
    ${UI_APP_HDRS} 
    ${RCC_APP_SRCS} 
    ${PROJECT_SOURCE_FILES} 
    ${RESOURCES_LIST}
)

if (MSVC)
    set_target_properties ( UIEditor PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"") #libc.lib;
endif()

target_link_libraries( UIEditor
    ${DAVA_LIBRARY}
    ${QT_LIBRARIES} 
    TextureCompression 
    TexturePacker 
    ${LIBRARIES}
)

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( UIEditor debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( UIEditor optimized ${FILE} )
endforeach ()





