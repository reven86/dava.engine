cmake_minimum_required (VERSION 3.0)

macro ( add_tool TARGET_NAME )
    cmake_parse_arguments ( ARG ""  "ROOT_DIR" "" ${ARGN} )

    set( INSTALL_DIR_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/app )

    if( ARG_ROOT_DIR )
        add_subdirectory      ( "${ARG_ROOT_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    else()
        add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    endif() 

    get_property( DEPLOY_DIR_PRJ GLOBAL PROPERTY DEPLOY_DIR_${TARGET_NAME} )

    add_dependencies( ${PROJECT_NAME} ${TARGET_NAME} )
endmacro ()

macro ( init_project )

    message("Creating " ${PROJECT_NAME} )

    set     ( DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME true )
    set     ( DAVA_USE_QT_WIDGETS true )
    set     ( DAVA_USE_RENDERSTATS true )
    set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" ) 
    set     ( TOOL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
    set     ( JOIN_PROJECT_NAME 1 )

    include ( CMake-common )
   
    set          ( DAVA_DISABLE_AUTOTESTS true )

    find_package ( DavaTools     REQUIRED )

    set          ( IGNORE_FILE_TREE_CHECK 1 )#tmp

    set          ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
    set          ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
    set          ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

endmacro()

############
project( ToolSet )


add_executable( ${PROJECT_NAME} MACOSX_BUNDLE main.cpp  )

if (DEPLOY)
    set ( DEPLOY_DIR_ROOT        ${CMAKE_BINARY_DIR}/app )
    set ( DEPLOY_DIR             ${DEPLOY_DIR_ROOT}/ToolSet.app/Contents )
    set ( DEPLOY_DIR_EXECUTABLE  ${DEPLOY_DIR_ROOT}/ToolSet.app/Contents/MacOS )
    set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR  ${CMAKE_BINARY_DIR}/app )
    add_definitions ( -DDATA_RELATIVE_DIRECTORY="../Data/" )
endif()

############

init_project ()
set          ( DAVA_MEGASOLUTION      1 )

## tools
set( MAC_DISABLE_BUNDLE true )

add_tool     ( ResourceEditor  )
add_tool     ( QuickEd         )

##
add_tool     ( ResourcePacker   )
add_tool     ( ResourcePatcher  )
add_tool     ( ImageUnpacker    )
add_tool     ( AssetCacheClient )
add_tool     ( AssetCacheServer )

set( MAC_DISABLE_BUNDLE )

## other
#add_tool     ( Launcher )

#add_tool     ( UnitTests       ROOT_DIR "${DAVA_ROOT_DIR}/Projects/UnitTests"   )
#add_tool     ( TestBed         ROOT_DIR "${DAVA_ROOT_DIR}/Projects/TestBed"     )
#add_tool     ( SceneViewer     ROOT_DIR "${DAVA_ROOT_DIR}/Projects/SceneViewer" )
#add_tool     ( PerfomanceTests ROOT_DIR "${DAVA_ROOT_DIR}/Projects/PerfomanceTests" )

if ( WIN32 )
    #add_tool ( UWPRunner )
endif ()

processing_mix_data()

get_property( QT5_FOUND GLOBAL PROPERTY QT5_FOUND )

if( QT5_FOUND )
    find_package( QT5 REQUIRED)
    qt_deploy( )
endif()




