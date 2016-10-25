cmake_minimum_required (VERSION 3.0)

project( ToolSet )

set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
include ( CMake-common )

macro ( add_tool TARGET_NAME )
    cmake_parse_arguments ( ARG ""  "ROOT_DIR" "" ${ARGN} )
    cmake_parse_arguments ( ARG ""  "CUSTOM_DEPLOY_DIR" "" ${ARGN} )

    set( OLD_DEPLOY_DIR ${DEPLOY_DIR} )
    if( ARG_CUSTOM_DEPLOY_DIR AND WIN32 )
        set( DEPLOY_DIR ${ARG_CUSTOM_DEPLOY_DIR}/${TARGET_NAME} )
    endif()  

    if( ARG_ROOT_DIR )
        add_subdirectory      ( "${ARG_ROOT_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    else()
        add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    endif() 

    set( DEPLOY_DIR ${OLD_DEPLOY_DIR} )
endmacro ()

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
set( DEFINITIONS           )
set( DEPLOY_DIR            ${CMAKE_BINARY_DIR}/app_other )
set( DEPLOY_DIR_EXECUTABLE )
set( QT_POST_DEPLOY        )
reset_property ( MIX_APP_DATA )

    add_tool     ( Launcher        CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
    add_tool     ( UnitTests       ROOT_DIR "${DAVA_ROOT_DIR}/Projects/UnitTests"       CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
    add_tool     ( TestBed         ROOT_DIR "${DAVA_ROOT_DIR}/Projects/TestBed"         CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
    add_tool     ( SceneViewer     ROOT_DIR "${DAVA_ROOT_DIR}/Projects/SceneViewer"     CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
    add_tool     ( PerfomanceTests ROOT_DIR "${DAVA_ROOT_DIR}/Projects/PerfomanceTests" CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )

    if ( WIN32 )
        add_tool ( UWPRunner CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
    endif ()
    processing_mix_data()

## tools

set( POSTPONED_MIX_DATA 1 )
set( QT_POST_DEPLOY 0 )

set( DEPLOY_DIR_MACOS             ${CMAKE_BINARY_DIR}/app )
set( DEPLOY_DIR_LIBS_MACOS        ${DEPLOY_DIR_MACOS}/Libs )
set( DEPLOY_DIR_DATA_MACOS        ${DEPLOY_DIR_MACOS} )
set( DEPLOY_DIR_EXECUTABLE_MACOS  ${DEPLOY_DIR_MACOS} )
set( ADDED_LD_RUNPATHES           "@executable_path/../../Libs @executable_path/../../../Libs" )


set( DEPLOY_DIR_WIN               ${CMAKE_BINARY_DIR}/app )
set( DEPLOY_DIR_LIBS_WIN          )
set( DEPLOY_DIR_DATA_WIN          )
set( DEPLOY_DIR_EXECUTABLE_WIN    )

set( DEPLOY_DIR             ${DEPLOY_DIR_${DAVA_PLATFORM_CURENT}})
set( DEPLOY_DIR_LIBS        ${DEPLOY_DIR_LIBS_${DAVA_PLATFORM_CURENT}})
set( DEPLOY_DIR_DATA        ${DEPLOY_DIR_DATA_${DAVA_PLATFORM_CURENT}})
set( DEPLOY_DIR_EXECUTABLE  ${DEPLOY_DIR_EXECUTABLE_${DAVA_PLATFORM_CURENT}})


reset_property ( MIX_APP_DATA )
    set( DEFINITIONS -DDAVA_MACOS_DATA_PATH="/../Data/" )
    add_tool     ( ResourceEditor   )
    add_tool     ( QuickEd          )
    set( DEFINITIONS  )

##
    add_tool     ( ResourceArchiver )
    add_tool     ( ResourcePacker   )
    add_tool     ( ResourcePatcher  )
    add_tool     ( ImageUnpacker    )
    add_tool     ( AssetCacheClient )
    add_tool     ( AssetCacheServer )

    processing_mix_data()

############





