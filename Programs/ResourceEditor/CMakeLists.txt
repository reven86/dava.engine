cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( ResourceEditor )

get_filename_component( FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

set                   ( WARNINGS_AS_ERRORS true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${FRAMEWORK_PATH}/Sources/CMake/Modules/" )
set                   ( TOOL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")

include ( CMake-common )

if (WIN32 AND DEPLOY AND NOT DAVA_MEGASOLUTION )
    set( DEPLOY_ROOT ${DEPLOY_DIR} )
    set( DEPLOY_ROOT ${DEPLOY_ROOT}/${PROJECT_NAME} )
    set( DEPLOY_DIR ${DEPLOY_ROOT}/dava.framework/Programs/${PROJECT_NAME} )
    set( DEPLOY_BEAST_DIR ${DEPLOY_DIR} )
endif()

set ( DAVA_COREV2 1 )

dava_add_definitions       (
-D__DAVAENGINE_COREV2__
)

find_package( DavaTools REQUIRED )


find_package( QT5 REQUIRED )
find_package( GMock REQUIRED )
find_package( QtHelpers REQUIRED )

find_package( DavaFramework REQUIRED COMPONENTS "Sound"
                                                DAVA_PLATFORM_QT 
                                                DAVA_ACQUIRE_OGL_CONTEXT_EVERYTIME 
                                                DAVA_DISABLE_AUTOTESTS 
                                                DAVA_USE_RENDERSTATS )

find_package( QtTools REQUIRED )
find_package( TArc REQUIRED )
find_package( RemoteTool REQUIRED )

#

include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}" )
include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}/Sources" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/include" )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/DavaTools/QtTools )

include_directories   ( ${DAVA_ROOT_DIR}/Programs/ColladaConverter/Collada15/FCollada )
include_directories   ( ${DAVA_ROOT_DIR}/Programs/ColladaConverter/Collada15/External/Cg/include )

include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH} )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet  )

include_directories   (.)
include_directories   ( "../TextureTools" )
include_directories   ( "Classes" )
include_directories   ( "Classes/Qt" )
include_directories   ( "Classes/Qt/CubemapEditor" )
include_directories   ( "Classes/Qt/TextureBrowser" )
include_directories   ( "Classes/Collada" )

dava_add_definitions       (
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
)

if (NOT PUBLIC_BUILD)
    dava_add_definitions(-D__DAVAENGINE_SPEEDTREE__)
endif()

if (DEPLOY)
    dava_add_definitions(-DDEPLOY_BUILD)
endif()

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

if( WIN32 )
	if (NOT PUBLIC_BUILD)
	    dava_add_definitions( -D__DAVAENGINE_BEAST__ )
	    set( BEAST_DIR ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR} )
        set( BEAST_WRAPPER "${BEAST_DIR}/wrapper" )
	    if( X64_MODE )
            set( BEAST_LIB "${BEAST_DIR}/beast/lib/beast64.lib" )
            set( BEAST_DLL "${BEAST_DIR}/beast/bin/beast64.dll")
	    else ()
            set( BEAST_LIB "${BEAST_DIR}/beast/lib/beast32.lib" )
            set( BEAST_DLL "${BEAST_DIR}/beast/bin/beast32.dll")
	    endif()
	endif()

	if ( X64_MODE )
        set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN64_release_bullet.lib )
    	set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN64_debug_bullet.lib )
	else ()
        set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN32_release_bullet.lib )
        set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN32_debug_bullet.lib )
	endif()


elseif( MACOS )
    set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/MACOS_release_bullet.a )    
    set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/MACOS_debug_bullet.a  )    

endif()

set ( Generated          ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source ( SOURCE "Classes" ${BEAST_WRAPPER} ${DAVA_SPEEDTREE_ROOT_DIR}/Sources "DataQt"
                GROUP_SOURCE Generated )

set( MIX_APP_DATA         "Data = ${DAVA_ROOT_DIR}/Programs/Data" 
                          "Data/${PROJECT_NAME} = ${CMAKE_CURRENT_LIST_DIR}/Data" )

set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns  )
set( ADDED_SRC            ${UI_APP_HDRS} ${RCC_APP_SRCS} ${QML_LIST} )
set( WIN32_RESOURCES            ${CMAKE_CURRENT_LIST_DIR}/WinSpecific/Resources.rc )
set( LIBRARIES            ${BEAST_LIB} )
set( LIBRARIES_RELEASE    ${THIRD_PARTY_LIB_RELEASE} )
set( LIBRARIES_DEBUG      ${THIRD_PARTY_LIB_DEBUG}   )

ASSERT(QT_ACTUAL_PATH "Qt path was not resolved")
set( ADDED_BINARY_DIR     ${QT_ACTUAL_PATH}/bin )
set( UNIFIED_IGNORE_LIST "Classes/Collada" )

convert_graphics()

setup_main_executable()


DEPLOY_SCRIPT (            COPY_WIN32  ${BEAST_DLL}
                           COPY_WIN64  ${BEAST_DLL}
                           COPY_DIR    ${DEPLOY_DIR})

if (DEPLOY OR WIN32 )
    if (NOT WIN32)
        set(BEAST_DIR "")
    endif()

    if( NOT DEPLOY AND WIN32 )
        set( DEPLOY_BEAST_DIR ${MIX_APP_DIR} )
    endif()

    if( NOT DEPLOY_BEAST_DIR )
        set( DEPLOY_BEAST_DIR ${DEPLOY_DIR} )
    endif()

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND "python" "${CMAKE_CURRENT_LIST_DIR}/Teamcity/postDeploy.py"
            "${DEPLOY_BEAST_DIR}"
            "${BEAST_DIR}"
        )
endif()
