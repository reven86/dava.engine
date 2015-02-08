cmake_minimum_required( VERSION 3.0 )

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( UIEditor )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include               ( CMake-common )

if ( MSVC )
    set( CMAKE_PREFIX_PATH    "${QT4_PATH_WIN}")
    set( QT_MOC_EXECUTABLE    "${QT4_PATH_WIN}/bin/moc.exe" )
    set( QT_RCC_EXECUTABLE    "${QT4_PATH_WIN}/bin/rcc.exe" )
    set( QT_UIC_EXECUTABLE    "${QT4_PATH_WIN}/bin/uic.exe" )
    set( QT_BINARY_DIR        "${QT4_PATH_WIN}/bin"         )
    message ( "QT4_PATH_WIN      -"  ${QT4_PATH_WIN}      )
    message ( "QT_MOC_EXECUTABLE -"  ${QT_MOC_EXECUTABLE} )
    message ( "QT_RCC_EXECUTABLE -"  ${QT_RCC_EXECUTABLE} )
    message ( "QT_UIC_EXECUTABLE -"  ${QT_UIC_EXECUTABLE} )
endif()

find_package          ( Qt4           REQUIRED )
find_package          ( DavaFramework REQUIRED )
include               ( ${QT_USE_FILE} )

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

file              ( GLOB_RECURSE QT_LIST "Classes/*.h" )
qt4_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "Ui/*.ui" )
qt4_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "Data/*.qrc" )
qt4_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

set ( Generated    ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set ( UiList       ${UI_LIST} )

define_source_folders          ( SRC_ROOT "Classes" GLOB_ERASE_FOLDERS "_build" )
generate_source_groups_project ( SRC_ROOT "Classes" GROUP_FOLDERS Generated UiList )
add_subdirectory               ( "${CMAKE_CURRENT_LIST_DIR}/../../Sources/Tools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )

set( MACOS_PLIST                ${CMAKE_CURRENT_LIST_DIR}/Info.plist )
set( MACOS_ICO                  ${CMAKE_CURRENT_LIST_DIR}/icon.icns )
set( MACOS_DATA                 ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_DYLIB                ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib )
set( ADDED_SRC                  ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( LIBRARIES                  ${QT_LIBRARIES} TextureTool )
set( LIBRARIES_RELEASE        )
set( LIBRARIES_DEBUG          )
set( ADDED_BINARY_DIR           ${QT_BINARY_DIR} )

setup_main_executable()

