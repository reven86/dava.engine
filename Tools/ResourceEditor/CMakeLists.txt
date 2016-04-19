cmake_minimum_required (VERSION 3.0)

set                   ( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

project               ( ResourceEditor )

set                   ( DAVA_DISABLE_AUTOTESTS true )
set                   ( DAVA_USE_RENDERSTATS   true )
set                   ( DAVA_USE_QT_WIDGETS    true )
set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
set                   ( QML_SCAN_DIR ${CMAKE_CURRENT_LIST_DIR} )
include               ( CMake-common )

add_definitions       ( -DDISABLE_NATIVE_WEBVIEW -DDISABLE_NATIVE_TEXTFIELD)

find_package          ( QT5           REQUIRED )
find_package          ( DavaFramework REQUIRED )
find_package          ( DavaTools     REQUIRED )

if (DEPLOY)
    set (DEPLOY_ROOT ${DEPLOY_DIR})

    if ( WIN32 )
        set(DEPLOY_ROOT ${DEPLOY_ROOT}/ResourceEditor)
        set(DEPLOY_DIR ${DEPLOY_ROOT}/dava.framework/Tools/ResourceEditor)
    endif()
endif()


include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}" )
include_directories   ( "${DAVA_SPEEDTREE_ROOT_DIR}/Sources" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
include_directories   ( "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/include" )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories   ( ${CMAKE_CURRENT_BINARY_DIR}/DavaTools/QtTools )

include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/FCollada )
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/External/Cg/include )

include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH} )
include_directories   ( ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet  )

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
)

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

if( WIN32 )
    add_definitions( -D__DAVAENGINE_BEAST__ )
    set( BEAST_WRAPPER   "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/wrapper" )
	if( X64_MODE )
    	set( BEAST_LIB       "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/lib/beast64.lib" )
	    set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN64_release_bullet.lib )    
    	set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN64_debug_bullet.lib )    
	else ()
    set( BEAST_LIB       "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/lib/beast32.lib" )
    set( THIRD_PARTY_LIB_RELEASE  ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN32_release_bullet.lib )    
    set( THIRD_PARTY_LIB_DEBUG    ${DAVA_THIRD_PARTY_ROOT_PATH}/bullet/Bin/WIN32_debug_bullet.lib )    
	endif()


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
set( ADDED_SRC            ${UI_APP_HDRS} ${RCC_APP_SRCS} )
set( LIBRARIES            ${BEAST_LIB} )
set( LIBRARIES_RELEASE    ${THIRD_PARTY_LIB_RELEASE} )
set( LIBRARIES_DEBUG      ${THIRD_PARTY_LIB_DEBUG}   )
                                                       
if ( WIN32 AND X64_MODE )
	set( ADDED_BINARY_DIR     ${QT5_PATH_WIN64}/bin )
else ()
	set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )
endif ()

set( UNIFIED_IGNORE_LIST "Classes/Collada" )

DEPLOY_SCRIPT            ( PYTHON      "${CMAKE_CURRENT_LIST_DIR}/DataSource/convert_graphics.py;${CMAKE_CURRENT_LIST_DIR}/DataSource"
                           COPY_WIN32  ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/bin/beast32.dll
                           COPY_WIN64  ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/bin/beast64.dll                           
                           COPY_DIR    ${DEPLOY_DIR}
                         )

setup_main_executable()
if (DEPLOY)
    set(BEAST_DIR ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR})
    if (NOT WIN32)
        set(BEAST_DIR "")
    endif()
    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND "python" "${CMAKE_CURRENT_LIST_DIR}/Teamcity/postDeploy.py"
            "${DEPLOY_ROOT}"
            "${BEAST_DIR}"
        )
endif()
