cmake_minimum_required (VERSION 3.0)

project( ToolSet )

set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include ( CMake-common )

macro ( add_tool TARGET_NAME )
    cmake_parse_arguments ( ARG ""  "ROOT_DIR;CUSTOM_DEPLOY_DIR;DEPLOY_DEFINE" "" ${ARGN} )

    if( ARG_DEPLOY_DEFINE AND DEPLOY ) 
        get_property( DEFINITIONS_OLD GLOBAL PROPERTY DEFINITIONS )
        append_property( DEFINITIONS ${ARG_DEPLOY_DEFINE} )
    endif()

    set( OLD_DEPLOY_DIR ${DEPLOY_DIR} )
    if( ARG_CUSTOM_DEPLOY_DIR AND WIN32 )
        set( DEPLOY_DIR ${ARG_CUSTOM_DEPLOY_DIR}/${TARGET_NAME} )
    endif()  

    if( ARG_ROOT_DIR )
        add_subdirectory      ( "${ARG_ROOT_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    else()
        add_subdirectory      ( "${DAVA_ROOT_DIR}/Programs/${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    endif() 

    if( ARG_DEPLOY_DEFINE AND DEPLOY ) 
        set_property( GLOBAL PROPERTY DEFINITIONS "${DEFINITIONS_OLD}" )
    endif()

    set( DEPLOY_DIR ${OLD_DEPLOY_DIR} )

    list( APPEND TOOL_TARGET_LIST ${TARGET_NAME} )

endmacro ()

############
set( UNITY_BUILD_OLD_VALUE ${UNITY_BUILD} )

set( JOIN_PROJECT_NAME 1 )
set( DAVA_MEGASOLUTION      1 )
set( IGNORE_FILE_TREE_CHECK 1 )

dava_add_definitions (-D__DAVAENGINE_COREV2__)

if( NOT DEPLOY_DIR )
    set( DEPLOY_DIR_MACOS             ${CMAKE_BINARY_DIR}/app )
    set( DEPLOY_DIR_WIN               ${CMAKE_BINARY_DIR}/app )
else()
    set( DEPLOY_DIR_MACOS             ${DEPLOY_DIR} )
    set( DEPLOY_DIR_WIN               ${DEPLOY_DIR} )
endif()

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

## other
set( DEFINITIONS            )

set( DEPLOY_DIR             ${CMAKE_BINARY_DIR}/app_other )
set( MIX_APP_DIR            )
set( DEPLOY_DIR_EXECUTABLE  )
set( QT_POST_DEPLOY         )
set( TOOL_TARGET_LIST       )

reset_property ( MIX_APP_DATA )

    if ( NOT ONLY_CONTENT_TOOLS )
         
        set( UNITY_BUILD false )
        add_tool     ( UnitTests       ROOT_DIR "${DAVA_ROOT_DIR}/Programs/UnitTests"       CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
        set( UNITY_BUILD ${UNITY_BUILD_OLD_VALUE} )

        add_tool     ( TestBed         ROOT_DIR "${DAVA_ROOT_DIR}/Programs/TestBed"         CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
        add_tool     ( SceneViewer     ROOT_DIR "${DAVA_ROOT_DIR}/Programs/SceneViewer"     CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
        add_tool     ( UIViewer     ROOT_DIR "${DAVA_ROOT_DIR}/Programs/UIViewer"     CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
        add_tool     ( PerfomanceTests ROOT_DIR "${DAVA_ROOT_DIR}/Programs/PerfomanceTests" CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
        add_tool     ( Launcher        CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

        if ( WIN32 )
            add_tool ( UWPRunner CUSTOM_DEPLOY_DIR ${DEPLOY_DIR} )
        endif ()
    endif ()

## tools

set( POSTPONED_MIX_DATA 1 )
set( QT_POST_DEPLOY 0 )

set( DEPLOY_DIR_LIBS_MACOS        ${DEPLOY_DIR_MACOS}/Libs )
set( DEPLOY_DIR_DATA_MACOS        ${DEPLOY_DIR_MACOS} )
set( DEPLOY_DIR_EXECUTABLE_MACOS  ${DEPLOY_DIR_MACOS} )
set( ADDED_LD_RUNPATHES           "@executable_path/../../Libs @executable_path/../../../Libs" )


set( DEPLOY_DIR_LIBS_WIN          )
set( DEPLOY_DIR_DATA_WIN          )
set( DEPLOY_DIR_EXECUTABLE_WIN    )

set( DEPLOY_DIR             ${DEPLOY_DIR_${DAVA_PLATFORM_CURENT}})
set( DEPLOY_DIR_LIBS        ${DEPLOY_DIR_LIBS_${DAVA_PLATFORM_CURENT}})
set( DEPLOY_DIR_DATA        ${DEPLOY_DIR_DATA_${DAVA_PLATFORM_CURENT}})
set( DEPLOY_DIR_EXECUTABLE  ${DEPLOY_DIR_EXECUTABLE_${DAVA_PLATFORM_CURENT}})
set( MIX_APP_DIR            ${CMAKE_BINARY_DIR}/MixResources )
set( TOOL_TARGET_LIST             )


reset_property ( MIX_APP_DATA )

    add_tool     ( ResourceEditor  DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )
    add_tool     ( QuickEd         DEPLOY_DEFINE "-DDAVA_MACOS_DATA_PATH=\"/../Data/\"" )

##
    add_tool     ( ResourceArchiver )
    add_tool     ( ResourcePacker   )
    add_tool     ( ResourcePatcher  )
    add_tool     ( ImageUnpacker    )
    add_tool     ( AssetCacheClient )
    add_tool     ( AssetCacheServer )
    add_tool     ( Dep )

    processing_mix_data()
    processing_mix_data_dependencies( "${TOOL_TARGET_LIST}" )

    dump_module_log()

############





