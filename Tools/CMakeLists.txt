cmake_minimum_required (VERSION 3.0)

macro ( add_tool TARGET_NAME )
    cmake_parse_arguments ( ARG ""  "ROOT_DIR" "" ${ARGN} )

    if( MACOS )
        set( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app )

    else()
        set( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app/${TARGET_NAME} )

    endif()  

    if( ARG_ROOT_DIR )
        add_subdirectory      ( "${ARG_ROOT_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    else()
        add_subdirectory      ( "${DAVA_ROOT_DIR}/Tools/${TARGET_NAME}" ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} )
    endif() 

    get_property( USE_NGT GLOBAL PROPERTY USE_NGT_${TARGET_NAME} )
    get_property( DEPLOY_DIR_PRJ GLOBAL PROPERTY DEPLOY_DIR_${TARGET_NAME} )

    if( USE_NGT AND DEPLOY )
        ADD_CUSTOM_COMMAND ( TARGET NGT_DEPLOY_Megasolution POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/app/Megasolution ${DEPLOY_DIR_PRJ}            
        )

        add_dependencies( NGT_DEPLOY_Megasolution ${TARGET_NAME}  )

    endif()

endmacro ()

project               ( Megasolution  )

set                   ( DAVA_USE_QT_WIDGETS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../Sources/CMake/Modules/" ) 
set                   ( TOOL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")

include               ( CMake-common )
include ( NGTMacro )

if (DEPLOY)
    set (DEPLOY_ROOT ${DEPLOY_DIR})

    if ( WIN32 )
        set(DEPLOY_ROOT ${DEPLOY_ROOT}/${PROJECT_NAME})
    endif()

    set (TOOL_OUTPUT_DIR ${DEPLOY_ROOT})
   
    add_custom_target ( NGT_DEPLOY_Megasolution ALL )

endif()

configure_ngt( PROJECT_NAME ${TOOL_OUTPUT_DIR} )
find_package ( NgtTools REQUIRED )

set                   ( DAVA_DISABLE_AUTOTESTS true )
find_package          ( DavaFramework REQUIRED COMPONENTS ALL )
find_package ( DavaTools     REQUIRED )

set          ( IGNORE_FILE_TREE_CHECK 1 )#tmp
set          ( DAVA_MEGASOLUTION      1 )

set          ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set          ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set          ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

add_tool     ( ResourceEditor   )
add_tool     ( QuickEd          )
add_tool     ( ResourcePacker   )
add_tool     ( ImageUnpacker    )
add_tool     ( AssetCacheClient )
add_tool     ( AssetCacheServer )

add_tool     ( UnitTests   ROOT_DIR "${DAVA_ROOT_DIR}/Projects/UnitTests"   )
add_tool     ( TestBed     ROOT_DIR "${DAVA_ROOT_DIR}/Projects/TestBed"     )
add_tool     ( SceneViewer ROOT_DIR "${DAVA_ROOT_DIR}/Projects/SceneViewer" )

if ( WIN32 )
    add_tool ( UWPRunner )
endif ()

