cmake_minimum_required (VERSION 2.8)
project ( ResourceEditor )

set          ( CMakeHelloWorld_VERSION_MAJOR 1 )
set          ( CMakeHelloWorld_VERSION_MINOR 0 )

set          ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 

find_package ( Qt4           REQUIRED )
find_package ( DavaFramework REQUIRED )

include      ( CMake-common )
include      ( ${QT_USE_FILE} )

set( SpeedTreeImporter  ${DAVA_SPEEDTREE_ROOT_DIR}/Sources )
set( BEAST              ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper )
set( Generated          ${CMAKE_CURRENT_BINARY_DIR}  )


define_source_folders ( SRC_ROOT "Classes" GLOB_ERASE_FOLDERS "_build" )
generate_source_groups_project ( GROUP_FOLDERS SpeedTreeImporter BEAST Generated Data )

include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}" )
include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}/Sources" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/include" )
include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib ) 
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/FCollada )
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/External/Cg/include )


include_directories   (.)
include_directories   ( "../TextureTools" )
include_directories   ( "Classes" )
include_directories   ( "Classes/bullet" )
include_directories   ( "Classes/Qt" )
include_directories   ( "Classes/Qt/CubemapEditor" )
include_directories   ( "Classes/Qt/TextureBrowser" )

include_directories   ( "Classes/Collada" )


add_definitions       ( 
-D__DAVAENGINE_SPEEDTREE__ 
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
#-D__DAVAENGINE_BEAST__
-DDAVA_FMOD
-D_DISABLE_IMAGE_MAGICK
-DDAVA_DEBUG
)

if( MSVC )
    set( LIBRARIES "gdi32.lib" "glu32.lib" #"winmm.lib" "opengl32.lib"
                   "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/lib/beast32.lib"  )

    set( LIBRARIES_DEBUG   "msvcrtd.lib"  )    
    set( LIBRARIES_RELEASE "msvcrt.lib"   ) 

endif()

set ( PROJECT_SOURCE_FILES ${PROJECT_SOURCE_FILES} "${DAVA_SPEEDTREE_ROOT_DIR}/Sources/SpeedTreeImporter.h" 
                                                   "${DAVA_SPEEDTREE_ROOT_DIR}/Sources/SpeedTreeImporter.cpp" )

file              ( GLOB_RECURSE QT_LIST "Classes/Qt/*.h"  )
qt4_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt4_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt4_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

add_subdirectory  ( "${CMAKE_CURRENT_LIST_DIR}/../../Sources/Tools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

if( MACOS )
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

    file ( GLOB_RECURSE RESOURCES_LIST "${CMAKE_CURRENT_LIST_DIR}/Data/*" )
    foreach ( FILE ${RESOURCES_LIST} )

        get_filename_component ( FILE_PATH ${FILE} PATH ) 
        STRING(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" FILE_GROUP ${FILE_PATH} )

        set_source_files_properties( ${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${FILE_GROUP} )
    endforeach ()


    file ( GLOB DYLIB_FILES ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*.dylib)
    list ( APPEND DYLIB_FILES   ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib ) 

    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks )

    list ( APPEND RESOURCES_LIST  ${DYLIB_FILES} ) 
    list ( APPEND LIBRARIES       ${DYLIB_FILES} )
    
elseif( WIN32 )
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP" ) 
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP" )

    file ( GLOB_RECURSE BEAST_LIST "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper/*.cpp"  
                                   "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper/*.h" 
                                  )
endif()

                               
add_executable  ( ${PROJECT_NAME} MACOSX_BUNDLE
    ${MOC_APP_SRCS} 
    ${UI_APP_HDRS} 
    ${RCC_APP_SRCS} 
    ${PROJECT_SOURCE_FILES} 
    "${DAVA_SPEEDTREE_ROOT_DIR}/Sources/SpeedTreeImporter.cpp"
    ${BEAST_LIST}
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

        COMMAND   
        install_name_tool -change ./libIMagickHelper.dylib @executable_path/../Frameworks/libIMagickHelper.dylib ${CMAKE_BINARY_DIR}/${BUILD_MODE}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}           
    )

elseif( MSVC )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib;libcmtd.lib\"")

endif()


target_link_libraries ( ${PROJECT_NAME}
    ${DAVA_LIBRARY}
    ${QT_LIBRARIES} 
    ${LIBRARIES} 
    TextureTool
)

if (MSVC) 
    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/editor.in
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )
endif()
 
foreach ( FILE ${LIBRARIES_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()

