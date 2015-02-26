cmake_minimum_required( VERSION 3.0 )
set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( QuickEd )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include               ( CMake-common )

find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )

include_directories   ( ${DAVA_INCLUDE_DIR} ) 
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib ) 
include_directories   (.)
include_directories   ( "Classes" )
include_directories   ( "Classes/Helpers" )
include_directories   ( "Classes/Controllers" )
include_directories   ( "Classes/Models" )

add_definitions       ( 
-DDAVA_QT
-DQT_HAVE_MMX
-DQT_HAVE_3DNOW
-DQT_HAVE_SSE
-DQT_HAVE_MMXEXT
-DQT_HAVE_SSE2
-DQT_LARGEFILE_SUPPORT
-DQT_THREAD_SUPPORT
)
                              
file                          ( GLOB_RECURSE QT_LIST "Classes/*.h" )
qt5_wrap_cpp                  ( MOC_APP_SRCS ${QT_LIST} )

file                          ( GLOB UI_LIST "Ui/*.ui" )
qt5_wrap_ui                   ( UI_APP_HDRS  ${UI_LIST} )

file                          ( GLOB RCC_LIST "Data/*.qrc" )
qt5_add_resources             ( RCC_APP_SRCS  ${RCC_LIST} )

set                           ( Generated    ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set                           ( UiList       ${UI_LIST} )

define_source_folders         ( SRC_ROOT "Classes" GLOB_ERASE_FOLDERS "_build"    )
generate_source_groups_project( SRC_ROOT "Classes" GROUP_FOLDERS Generated UiList )
add_subdirectory              ( "${CMAKE_CURRENT_LIST_DIR}/../../Sources/Tools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )
 
set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO                  ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns )
set( MACOS_DATA                 ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_DYLIB                ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib )
set( ADDED_SRC                  ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( LIBRARIES                  ${QT_LIBRARIES} TextureTool )
set( LIBRARIES_RELEASE        )
set( LIBRARIES_DEBUG          )
set( ADDED_BINARY_DIR           ${QT5_PATH_WIN}/bin )
set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )

setup_main_executable()

if( WIN32 )
    set( OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/$(Configuration)" )
    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR} )
    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

endif()

get_filename_component(version_file "../../gitTime.txt" ABSOLUTE)
if( EXISTS ${version_file})
    file (STRINGS ${version_file} BUILD_VERION)
    add_definitions( -DUI_EDITOR_VERSION=\"${BUILD_VERION}\" )   

endif()
