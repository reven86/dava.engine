cmake_minimum_required( VERSION 3.2.3 )

set     ( WINDOWS_UAP 1 )
set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../Modules/" ) 
include ( GlobalVariables )

#define system name and version for windows universal application
set ( CMAKE_SYSTEM_NAME "${WINDOWS_UAP_PLATFORM_NAME}" )
set ( CMAKE_SYSTEM_VERSION "${WINDOWS_UAP_PLATRORM_VERSION}" )
set ( CMAKE_VS_EFFECTIVE_PLATFORMS "Win32;ARM;x64" )