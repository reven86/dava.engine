cmake_minimum_required( VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( ResourceEditor )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include               ( CMake-common )

find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )

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
-DDAVA_FMOD
-D_DISABLE_IMAGE_MAGICK
)


file              ( GLOB_RECURSE QT_LIST "Classes/Qt/*.h"  )
qt5_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

if( MSVC )
    add_definitions( -D__DAVAENGINE_BEAST__ )
    set( BEAST_WRAPPER "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
    set( BEAST_LIB     "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/lib/beast32.lib" )

endif()

set ( SpeedTreeImporter  ${DAVA_SPEEDTREE_ROOT_DIR}/Sources )
set ( BEAST              ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper )
set ( Generated          ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source_folders          ( SRC_ROOT "Classes" ${BEAST_WRAPPER} ${DAVA_SPEEDTREE_ROOT_DIR}/Sources )
generate_source_groups_project ( SRC_ROOT "Classes" "DataQt" GROUP_FOLDERS SpeedTreeImporter BEAST Generated  )
add_subdirectory               ( "${CMAKE_CURRENT_LIST_DIR}/../../Sources/Tools" ${CMAKE_CURRENT_BINARY_DIR}/TextureTools )


set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns  )
set( MACOS_DATA           ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_DYLIB          ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib )
set( ADDED_SRC            ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS}  )
set( LIBRARIES            ${BEAST_LIB} TextureTool )
set( LIBRARIES_RELEASE    ${QT_LIBRARIES} )
set( LIBRARIES_DEBUG      ${QT_LIBRARIES} )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

setup_main_executable()



