# Only interpret ``if()`` arguments as variables or keywords when unquoted.
#if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
#endif()

#
function ( load_config CONFIG_FILE )

    file( STRINGS ${CONFIG_FILE} ConfigContents )
    foreach( NameAndValue ${ConfigContents} )
        string( REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue} )
        string( REGEX MATCH "^[^=]+" Name ${NameAndValue} )
        string( REPLACE "${Name}=" "" Value ${NameAndValue} )
        string( STRIP "${Name}" Name)
        string( STRIP "${Value}" Value)
        if( NOT ${Name} )
            set( ${Name} "${Value}" PARENT_SCOPE )
        endif()
        #  message("---" [${Name}] "  " [${Value}] )
    endforeach()

endfunction ()

if( APPLE AND NOT IOS AND NOT ANDROID )
    set( MACOS 1 )
endif ()

# Detect linux platform by comparing CMAKE_SYSTEM_NAME value with "Linux" excluding android
# as cross-compiling is possible.
# cmake's variable UNIX is not suitable as it is defined for all unix-like system
# including macos, android, linux.
if (CMAKE_SYSTEM_NAME AND ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux") AND NOT ANDROID)
    set (LINUX 1)
endif()

if( TEAMCITY_DEPLOY )
    set( OUTPUT_TO_BUILD_DIR true )
    set( IGNORE_FILE_TREE_CHECK true )
    set( DEBUG_INFO true )

endif()

#constants
set( DAVA_ANDROID_MAX_LIB_SRC 700 )

#global paths
set( DAVA_ROOT_DIR                      "${CMAKE_CURRENT_LIST_DIR}/../../.." )
set( DAVA_PREDEFINED_TARGETS_FOLDER     "CMAKE" )

get_filename_component( DAVA_ROOT_DIR ${DAVA_ROOT_DIR} ABSOLUTE )

set( DAVA_BIN_DIR "${DAVA_ROOT_DIR}/Bin" )

if( WIN32  )
	string ( FIND ${CMAKE_GENERATOR} "Win64" X64_PROJECT )

	if( ${X64_PROJECT} EQUAL -1 )
		set ( X64_MODE false )
                set( DAVA_PROJECT_BIT 32 )
	else ()
		set ( X64_MODE true )
                set( DAVA_PROJECT_BIT 64 )
	endif ()
endif()

################

set( DAVA_PLATFORM_LIST IOS 
                        MACOS 
                        ANDROID 
                        WIN 
                        WINUAP
                        LINUX
                        )

if( IOS )
    set( DAVA_PLATFORM_CURENT IOS )
elseif( MACOS )
    set( DAVA_PLATFORM_CURENT MACOS )
elseif( ANDROID )
    set( DAVA_PLATFORM_CURENT ANDROID )
elseif( WIN32 AND NOT WINDOWS_UAP )
    set( DAVA_PLATFORM_CURENT WIN )
    set( WIN true )
elseif( WIN32 AND WINDOWS_UAP )
    set( DAVA_PLATFORM_CURENT WINUAP )
    set( WINUAP true )
elseif (LINUX)
    set( DAVA_PLATFORM_CURENT LINUX )
endif()


set( DAVA_MODULES_DIR                   "${DAVA_ROOT_DIR}/Modules" )
set( DAVA_SOURCES_DIR                   "${DAVA_ROOT_DIR}/Sources" )
set( DAVA_TOOLS_DIR                     "${DAVA_SOURCES_DIR}/Programs" )
set( DAVA_ENGINE_DIR                    "${DAVA_SOURCES_DIR}/Internal" )
set( DAVA_EXTERNAL_DIR                  "${DAVA_SOURCES_DIR}/External" )
set( DAVA_PLATFORM_SRC                  "${DAVA_ENGINE_DIR}/Platform" )
set( DAVA_THIRD_PARTY_ROOT_PATH         "${DAVA_ROOT_DIR}/Libs" )
set( DAVA_CONFIGURE_FILES_PATH          "${DAVA_ROOT_DIR}/Sources/CMake/ConfigureFiles" )
set( DAVA_MODULES_FILES_PATH            "${DAVA_ROOT_DIR}/Sources/CMake/Modules" )
set( DAVA_SCRIPTS_FILES_PATH            "${DAVA_ROOT_DIR}/Sources/CMake/Scripts" )
set( DAVA_THIRD_PARTY_INCLUDES_PATH     "${DAVA_THIRD_PARTY_ROOT_PATH}/include" 
                                        "${DAVA_ENGINE_DIR}/../External"
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/glew/include" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/icucommon/source/common" 
                                      ) 

set( DAVA_SPEEDTREE_ROOT_DIR            "${DAVA_ROOT_DIR}/../dava.speedtree" )                                      
set( DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast" ) 

#additional variables for Windows UAP
if ( WINDOWS_UAP )
    #turning on openssl_WinRT lib on Windows Store
    set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/openssl/include/uwp"
    )
  
    #Deprecated since cmake 3.4, added for backwards compatibility
    set ( CMAKE_VS_TARGET_PLATFORM_VERSION ${WINDOWS_UAP_TARGET_PLATFORM_VERSION} )

    #set extensions version
    set ( WINDOWS_UAP_MOBILE_EXT_SDK_VERSION ${WINDOWS_UAP_TARGET_PLATFORM_VERSION} )
    set ( WINDOWS_UAP_IOT_EXT_SDK_VERSION    ${WINDOWS_UAP_TARGET_PLATFORM_VERSION} )
    
elseif ( WIN32 )
    if ( X64_MODE )
        set ( INC_ARCH "x64" )
    else ()
        set ( INC_ARCH "x86" )
    endif ()
    
    set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}"
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/openssl/include/win32/${INC_ARCH}" )
                                        
elseif ( ANDROID )
    set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}"
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/openssl/include/android" )

endif()

# Openssl includes
set ( DAVA_OPENSSL_ARCH "." )
if ( WINDOWS_UAP )
    set ( DAVA_OPENSSL_PLATFORM "uwp" )
    
elseif ( WIN32 )
    set ( DAVA_OPENSSL_PLATFORM "win32" )
    if ( X64_MODE )
        set ( DAVA_OPENSSL_ARCH "x64" )
    else ()
        set ( DAVA_OPENSSL_ARCH "x86" )
    endif ()
    
elseif ( ANDROID )
    set ( DAVA_OPENSSL_PLATFORM "android" )

elseif ( MACOS )
    set ( DAVA_OPENSSL_PLATFORM "mac" )
    
elseif ( IOS )
    set ( DAVA_OPENSSL_PLATFORM "ios" )

elseif ( LINUX )
    set ( DAVA_OPENSSL_PLATFORM "linux" )
else ()
    message ( FATAL_ERROR "Unknown platform" )
    
endif ()

set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}"
                                    "${DAVA_THIRD_PARTY_ROOT_PATH}/openssl/include/${DAVA_OPENSSL_PLATFORM}/${DAVA_OPENSSL_ARCH}" )

get_filename_component( DAVA_SPEEDTREE_ROOT_DIR ${DAVA_SPEEDTREE_ROOT_DIR} ABSOLUTE )
get_filename_component( DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR} ABSOLUTE )

set( DAVA_BINARY_WIN32_DIR  "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/bin" )

set( DAVA_INCLUDE_DIR       ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_INCLUDES_PATH} )


if( NOT DEPLOY_DIR )
    set ( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app )
endif()

if( MACOS AND NOT MAC_DISABLE_BUNDLE)
    set( DEPLOY_EXECUTE_DIR ${DEPLOY_DIR}/${PROJECT_NAME}.app/Contents/MacOS )
else()
    set( DEPLOY_EXECUTE_DIR ${DEPLOY_DIR} )
endif()


#DavaConfig
if( CUSTOM_DAVA_CONFIG_PATH_MAC AND MACOS )
    set(CUSTOM_DAVA_CONFIG_PATH ${CUSTOM_DAVA_CONFIG_PATH_MAC} )
endif()

if( CUSTOM_DAVA_CONFIG_PATH_WIN AND WIN32 )
    set(CUSTOM_DAVA_CONFIG_PATH ${CUSTOM_DAVA_CONFIG_PATH_WIN} )    
endif()

if( CUSTOM_DAVA_CONFIG_PATH  )
    set( DAVA_CONFIG_PATH ${CUSTOM_DAVA_CONFIG_PATH} )

else()
    set( DAVA_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR}/config.in )
    
    if( NOT EXISTS "${DAVA_ROOT_DIR}/DavaConfig.in")
        configure_file( ${DAVA_CONFIGURE_FILES_PATH}/DavaConfigTemplate.in
                        ${DAVA_ROOT_DIR}/DavaConfig.in COPYONLY )

    endif()

    configure_file( ${DAVA_ROOT_DIR}/DavaConfig.in 
                    ${DAVA_CONFIG_PATH} )

endif()

load_config ( ${DAVA_CONFIG_PATH} )

if( ANDROID_NDK_TYPE )
    set( ANDROID_NDK  ${ANDROID_NDK_${ANDROID_NDK_TYPE}} )
endif()

