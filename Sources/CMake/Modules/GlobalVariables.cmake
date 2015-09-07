#
macro ( load_config CONFIG_FILE )

    file( STRINGS ${CONFIG_FILE} ConfigContents )
    foreach( NameAndValue ${ConfigContents} )
        string( REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue} )
        string( REGEX MATCH "^[^=]+" Name ${NameAndValue} ) 
        string( REPLACE "${Name}=" "" Value ${NameAndValue} )
        string( REGEX REPLACE " " "" Name   "${Name}" )
        string( REGEX REPLACE " " "" Value  "${Value}" )
        if( NOT ${Name} )
            set( ${Name} "${Value}" )
        endif()
        #  message("---" [${Name}] "  " [${Value}] )
    endforeach()

endmacro ()

# Only interpret ``if()`` arguments as variables or keywords when unquoted.
if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
endif()

if( APPLE AND NOT IOS AND NOT ANDROID )
	set( MACOS 1 )
endif ()

if( TEAMCITY_DEPLOY )
    set( OUTPUT_TO_BUILD_DIR true )
    set( IGNORE_FILE_TREE_CHECK true )
    set( DEBUG_INFO true )

endif()

#constants
set( DAVA_ANDROID_MAX_LIB_SRC 700 )

#global paths
set( DAVA_LIBRARY                       "DavaFramework" )
set( DAVA_ROOT_DIR                      "${CMAKE_CURRENT_LIST_DIR}/../../.." )
set( DAVA_PREDEFINED_TARGETS_FOLDER     "CMAKE" )

get_filename_component( DAVA_ROOT_DIR ${DAVA_ROOT_DIR} ABSOLUTE )

set( DAVA_TOOLS_BIN_DIR                 "${DAVA_ROOT_DIR}/Tools/Bin" )
set( DAVA_TOOLS_DIR                     "${DAVA_ROOT_DIR}/Sources/Tools" )
set( DAVA_ENGINE_DIR                    "${DAVA_ROOT_DIR}/Sources/Internal" )
set( DAVA_PLATFORM_SRC                  "${DAVA_ENGINE_DIR}/Platform" )
set( DAVA_THIRD_PARTY_ROOT_PATH         "${DAVA_ROOT_DIR}/Libs" )
set( DAVA_CONFIGURE_FILES_PATH          "${DAVA_ROOT_DIR}/Sources/CMake/ConfigureFiles" )
set( DAVA_SCRIPTS_FILES_PATH            "${DAVA_ROOT_DIR}/Sources/CMake/Scripts" )
set( DAVA_THIRD_PARTY_INCLUDES_PATH     "${DAVA_THIRD_PARTY_ROOT_PATH}/include" 
                                        "${DAVA_ENGINE_DIR}/../External" 
                                        "${DAVA_ENGINE_DIR}/../Tools" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/glew/include" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/fmod/include" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/lua/include" 
                                      ) 

set( DAVA_SPEEDTREE_ROOT_DIR            "${DAVA_ROOT_DIR}/../dava.speedtree" )                                      
set( DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast" ) 

#additional variables for Windows UAP
if ( WINDOWS_UAP )
    #turning on ANGLE and openssl_WinRT lib on Windows Store
    set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}" 
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/angle-ms-master/include"
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/openssl_win10/include"
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/fmod_uap/include" )

    #libs paths	
    set ( DAVA_WIN_UAP_LIBRARIES_PATH_COMMON "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win10" ) 
    if( "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "ARM" )
        set ( LIBS_ARCH_SELECTOR "arm" )
    elseif ( "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64" )
        set ( LIBS_ARCH_SELECTOR "x64" )
    else ()
        set ( LIBS_ARCH_SELECTOR "x86" )
    endif ()

    set ( DAVA_WIN_UAP_LIBRARIES_PATH_DEBUG   "${DAVA_WIN_UAP_LIBRARIES_PATH_COMMON}/${LIBS_ARCH_SELECTOR}/Debug" )
    set ( DAVA_WIN_UAP_LIBRARIES_PATH_RELEASE "${DAVA_WIN_UAP_LIBRARIES_PATH_COMMON}/${LIBS_ARCH_SELECTOR}/Release" )
    
else ()
    set( DAVA_THIRD_PARTY_INCLUDES_PATH "${DAVA_THIRD_PARTY_INCLUDES_PATH}"
                                        "${DAVA_THIRD_PARTY_ROOT_PATH}/openssl/includes" )

endif()
                                   
get_filename_component( DAVA_SPEEDTREE_ROOT_DIR ${DAVA_SPEEDTREE_ROOT_DIR} ABSOLUTE )
get_filename_component( DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR} ABSOLUTE )

set( DAVA_BINARY_WIN32_DIR  "${DAVA_TOOLS_BIN_DIR}" "${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR}/beast/bin"  )
set( DAVA_INCLUDE_DIR       ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_INCLUDES_PATH} ${DAVA_TOOLS_DIR} )

if( NOT DEPLOY_DIR )
    set ( DEPLOY_DIR ${CMAKE_BINARY_DIR}/app )

endif()

#DavaConfig
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