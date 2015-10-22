cmake_minimum_required( VERSION 3.2.3 )

set     ( WINDOWS_UAP true )
set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../Modules/" ) 
include ( GlobalVariables )

#define system name and version for windows universal application
set ( CMAKE_SYSTEM_NAME "WindowsStore" )
set ( CMAKE_SYSTEM_VERSION "10.0" )

#project platforms
#if concrete platform not set, project will be multiplatform
#else project will be singleplatform
if ( NOT CMAKE_GENERATOR_PLATFORM )
    set ( CMAKE_VS_EFFECTIVE_PLATFORMS "Win32;ARM;x64" )
    set ( WINDOWS_UAP_MULTIPLATFORM true )
    set ( WINDOWS_UAP_PLATFORMS ${CMAKE_VS_EFFECTIVE_PLATFORMS} )
else ()
    set ( WINDOWS_UAP_PLATFORMS ${CMAKE_GENERATOR_PLATFORM} )
endif ()

#extensions
set ( WINDOWS_UAP_MOBILE_EXT_SDK_VERSION "10.0.10240.0" )
set ( WINDOWS_UAP_IOT_EXT_SDK_VERSION "10.0.10240.0" )