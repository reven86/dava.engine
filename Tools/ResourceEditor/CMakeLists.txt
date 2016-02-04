cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( ResourceEditor )

get_filename_component( FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

set                   ( DAVA_DISABLE_AUTOTESTS true )
set                   ( DAVA_USE_RENDERSTATS   true )
set                   ( DAVA_USE_QT_WIDGETS    true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${FRAMEWORK_PATH}/Sources/CMake/Modules/" )
set                   ( USE_DYNAMIC_CRT 1)
set                   ( TARGET_PLUGIN_APP_FOLDER_NAME ${PROJECT_NAME})
set                   ( TOOL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")

if (DEPLOY_DIR)
      set (TOOL_OUTPUT_DIR ${DEPLOY_DIR})
endif()

include               ( CMake-common )
include               ( NGTMacro )


define_ngt_pathes("${FRAMEWORK_PATH}/Sources/External/ngt/")
get_ngt_modules(NGT_LIBS NGT_PLUGINS QT_COMPONENTS)

add_definitions(
      -DDAVA_MEMORY_MANAGEMENT
)

list(APPEND EXCLUDE_LIBS "FColladaVS2010.lib" "libdxt.lib" "libdxtd.lib")
list(APPEND ADDITIONAL_RELEASE_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/FColladaVS2010.lib")
list(APPEND ADDITIONAL_DEBUG_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/D/FColladaVS2010.lib")
list(APPEND ADDITIONAL_RELEASE_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/libdxt.lib")
list(APPEND ADDITIONAL_DEBUG_LIBS "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/NGT/D/libdxtd.lib")

find_package          ( QT5           REQUIRED COMPONENTS ${QT_COMPONENTS} )
find_package          ( NGT REQUIRED )
find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )

if (NOT DEPLOY_DIR)
      BW_SET_USE_CONFIGURATION_NAME(1)
endif()
BW_SET_TEST_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_ASSET_PIPELINE_DST_DIR(${TOOL_OUTPUT_DIR})
BW_SET_EXECUTABLE_DST_DIR(${TOOL_OUTPUT_DIR})

# NGT build system use this variable on Mac to resolve path where plugins must be copyed
set( BW_BUNDLE_NAME "ResourceEditor")

include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}" )
include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}/Sources" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/include" )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/DavaTools/QtTools )

include_directories   ( ${DAVA_ROOT_DIR}/Sources/Tools)
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/FCollada )
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/External/Cg/include )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib )

include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH} )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet )

include_directories   (.)
include_directories   ( "../TextureTools" )
include_directories   ( "Classes" )
include_directories   ( "Classes/Qt" )
include_directories   ( "Classes/Qt/CubemapEditor" )
include_directories   ( "Classes/Qt/TextureBrowser" )
include_directories   ( "Classes/Collada" )

add_definitions       (
-D__DAVAENGINE_SPEEDTREE__
-D__DAVAENGINE_RENDERSTATS__
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

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

if( WIN32 )
    add_definitions( -D__DAVAENGINE_BEAST__ )
    set( BEAST_WRAPPER   "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
    set( BEAST_LIB       "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/lib/beast32.lib" )

    set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN32_release_bullet.lib )    
    set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN32_debug_bullet.lib )    

elseif( MACOS )
    set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/MACOS_release_bullet.a )    
    set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/MACOS_debug_bullet.a  )    

endif()

set ( SpeedTreeImporter  ${DAVA_SPEEDTREE_ROOT_DIR}/Sources )
set ( BEAST              ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper )
set ( Generated          ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source_folders          ( SRC_ROOT "Classes" ${BEAST_WRAPPER} ${DAVA_SPEEDTREE_ROOT_DIR}/Sources )
generate_source_groups_project ( SRC_ROOT "Classes" "DataQt" GROUP_FOLDERS SpeedTreeImporter BEAST Generated  )

set( APP_DATA             ${CMAKE_CURRENT_LIST_DIR}/Data )
set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns  )
set( MACOS_DYLIB          ${DAVA_THIRD_PARTY_ROOT_PATH}/IMagickHelpLib/libIMagickHelper.dylib )
set( ADDED_SRC            ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( LIBRARIES            ${BEAST_LIB} )
set( LIBRARIES_RELEASE    ${THIRD_PARTY_LIB_RELEASE} )
set( LIBRARIES_DEBUG      ${THIRD_PARTY_LIB_DEBUG}   )
                                                       

set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

set( UNIFIED_IGNORE_LIST "Classes/Collada" )

DEPLOY_SCRIPT            ( PYTHON      "${CMAKE_CURRENT_LIST_DIR}/DataSource/convert_graphics.py;${CMAKE_CURRENT_LIST_DIR}/DataSource"
                           COPY_WIN32  ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/bin/beast32.dll
                           COPY_DIR    ${DEPLOY_DIR}
                         )

if (BUILD_AS_PLUGIN)
    add_definitions(-D__DAVAENGINE_NGT_PLUGIN__)
endif()

# append NGT targets to solution
foreach(NGT_LIB ${NGT_LIBS})
    append_ngt_lib(${NGT_LIB})
endforeach()

foreach(NGT_PLUGIN ${NGT_PLUGINS})
    append_ngt_plugin(${NGT_PLUGIN})
endforeach()

list(APPEND TARGET_LIBRARIES core_generic_plugin_manager
                                                              core_generic_plugin
                                                              core_qt_common
                                                              core_ui_framework)

setup_main_executable()
