cmake_minimum_required (VERSION 3.0)

set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include ( CMake-common )

macro ( add_tool TARGET_NAME )
    cmake_parse_arguments ( ARG ""  "ROOT_DIR" "" ${ARGN} )

    set( INSTALL_DIR_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/app )

    if( ARG_ROOT_DIR )
        add_subdirectory      ( "${ARG_ROOT_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    else()
        add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    endif() 

    get_property( DEPLOY_DIR_PRJ GLOBAL PROPERTY DEPLOY_DIR_${TARGET_NAME} )
    list( APPEND TARGETS_LIST ${TARGET_NAME} )
endmacro ()

macro ( init_tool_set )
    project( ToolSet )

    set( MACOS_PLIST            ${CMAKE_CURRENT_LIST_DIR}/Info.plist )
    set( MACOS_ICO              ${CMAKE_CURRENT_LIST_DIR}/icon.icns  )
    set( RESOURCES_LIST         ${MACOS_PLIST} ${MACOS_ICO} )

    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE main.cpp ${RESOURCES_LIST} )

    set_target_properties ( ${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE_INFO_PLIST "${MACOS_PLIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                            RESOURCE "${MACOS_ICO}"
                            XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR ${CMAKE_BINARY_DIR}/app
                          )

    add_dependencies( ${PROJECT_NAME} ${TARGETS_LIST} )
    set( MACOS_PLIST )
    set( MACOS_ICO )
    set( RESOURCES_LIST )

    processing_mix_data()
    get_property( QT5_FOUND GLOBAL PROPERTY QT5_FOUND )
    if( QT5_FOUND )
        find_package( QT5 REQUIRED)
        qt_deploy( )
    endif()
endmacro()

############

set( DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME true )
set( DAVA_USE_QT_WIDGETS true )
set( DAVA_USE_RENDERSTATS true )
set( JOIN_PROJECT_NAME 1 )
set( DAVA_DISABLE_AUTOTESTS true )
set( DAVA_MEGASOLUTION      1 )
set( IGNORE_FILE_TREE_CHECK 1 )

find_package ( DavaTools     REQUIRED )

## other
set( MAC_DISABLE_BUNDLE    )
set( DEFINITIONS           )
set( DEPLOY_DIR            ${CMAKE_BINARY_DIR}/app/other )
set( DEPLOY_DIR_EXECUTABLE )
reset_property ( MIX_APP_DATA )

    add_tool     ( Launcher )

    add_tool     ( UnitTests       ROOT_DIR "${DAVA_ROOT_DIR}/Projects/UnitTests"   )
    add_tool     ( TestBed         ROOT_DIR "${DAVA_ROOT_DIR}/Projects/TestBed"     )
    add_tool     ( SceneViewer     ROOT_DIR "${DAVA_ROOT_DIR}/Projects/SceneViewer" )
    add_tool     ( PerfomanceTests ROOT_DIR "${DAVA_ROOT_DIR}/Projects/PerfomanceTests" )

    if ( WIN32 )
        add_tool ( UWPRunner )
    endif ()
    processing_mix_data()

## tools
set( MAC_DISABLE_BUNDLE true )
set( DEFINITIONS -DDATA_RELATIVE_DIRECTORY="../Resources/Data/" )
set( DEPLOY_DIR             ${CMAKE_BINARY_DIR}/app )
set( DEPLOY_DIR_LIBS        ${DEPLOY_DIR}/ToolSet.app/Contents/Libs )
set( DEPLOY_DIR_DATA        ${DEPLOY_DIR}/ToolSet.app/Contents/Resources )
set( DEPLOY_DIR_EXECUTABLE  ${DEPLOY_DIR}/ToolSet.app/Contents/MacOS )
set( TARGETS_LIST )
set( POSTPONED_MIX_DATA 1 )

reset_property ( MIX_APP_DATA )

    add_tool     ( ResourceEditor  )
    add_tool     ( QuickEd         )
##
    add_tool     ( ResourcePacker   )
    add_tool     ( ResourcePatcher  )
    add_tool     ( ImageUnpacker    )
    add_tool     ( AssetCacheClient )
    add_tool     ( AssetCacheServer )

    init_tool_set()

############





