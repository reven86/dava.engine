cmake_minimum_required (VERSION 2.8)
project      ( UIEditor )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

find_package ( Qt4           REQUIRED )
find_package ( DavaFramework REQUIRED )

include      ( CMake-common )
include      ( ${QT_USE_FILE} )

define_source_folders ( SRC_ROOT "Classes" GLOB_ERASE_FOLDERS "_build" )

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib ) 
include_directories   (.)
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

add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../../Sources/Tools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

generate_source_groups_project ()
                               
add_executable  ( UIEditor MACOSX_BUNDLE
    ${MOC_APP_SRCS} 
    ${UI_APP_HDRS} 
    ${RCC_APP_SRCS} 
    ${PROJECT_SOURCE_FILES} 
    ${RESOURCES_LIST}
)

if( APPLE )

    set( BUILD_MODE $<CONFIG> ) #$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release> )

    ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND   
        install_name_tool -change ./libfmodex.dylib    @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/Frameworks/libfmodevent.dylib  

        COMMAND   
        install_name_tool -change ./libfmodevent.dylib @executable_path/../Frameworks/libfmodevent.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}    

        COMMAND   
        install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}    
    )


elseif ( MSVC )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"")    
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/editor.in
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )
endif()

target_link_libraries( ${PROJECT_NAME}
    ${DAVA_LIBRARY}
    ${QT_LIBRARIES} 
    ${LIBRARIES}
    TextureTool
)

foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()





