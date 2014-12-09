
if ( GLOBAL_VAR_FOUND )
    return ()
endif ()
set ( GLOBAL_VAR_FOUND 1 )

if( APPLE AND NOT IOS )
	set ( MACOS 1 )
endif ()

set ( DAVA_LIBRARY                     "DavaFramework" )
set ( DAVA_ROOT_DIR                    "${CMAKE_CURRENT_LIST_DIR}/../../.." )

get_filename_component( DAVA_ROOT_DIR ${DAVA_ROOT_DIR} ABSOLUTE )

set ( DAVA_TOOLS_BIN_DIR               "${DAVA_ROOT_DIR}/Tools/Bin" )
set ( DAVA_TOOLS_DIR                   "${DAVA_ROOT_DIR}/Sources/Tools" )
set ( DAVA_ENGINE_DIR                  "${DAVA_ROOT_DIR}/Sources/Internal" )
set ( DAVA_THIRD_PARTY_ROOT_PATH       "${DAVA_ROOT_DIR}/Libs" )
set ( DAVA_CONFIGURE_FILES_PATH        "${DAVA_ROOT_DIR}/Sources/CMake/ConfigureFiles" )
set ( DAVA_THIRD_PARTY_INCLUDES_PATH   "${DAVA_THIRD_PARTY_ROOT_PATH}/include" 
                                       "${DAVA_ENGINE_DIR}/../External" 
                                       "${DAVA_ENGINE_DIR}/../Tools" 
                                       "${DAVA_THIRD_PARTY_ROOT_PATH}/glew/include" 
                                       "${DAVA_THIRD_PARTY_ROOT_PATH}/fmod/include" 
                                      ) 

set( DAVA_SPEEDTREE_ROOT_DIR            "${DAVA_ROOT_DIR}/../dava.speedtree" )                                      
set( DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR "${DAVA_ROOT_DIR}/../dava.resourceeditor.beast" ) 
                                   
get_filename_component( DAVA_SPEEDTREE_ROOT_DIR ${DAVA_SPEEDTREE_ROOT_DIR} ABSOLUTE )
get_filename_component( DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR ${DAVA_RESOURCEEDITOR_BEAST_ROOT_DIR} ABSOLUTE )

if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/ios" ) 
    
elseif ( MACOS )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac" ) 

else   ()
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win" ) 
    
endif  ()

set ( DAVA_INCLUDE_DIR ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

configure_file( ${DAVA_ROOT_DIR}/DavaConfig.in 
                ${CMAKE_CURRENT_BINARY_DIR}/DavaConfig.in )

file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/DavaConfig.in ConfigContents)
foreach(NameAndValue ${ConfigContents})
  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
  string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
  string(REPLACE "${Name}=" "" Value ${NameAndValue})
  string(REGEX REPLACE " " "" Name   "${Name}")
  string(REGEX REPLACE " " "" Value  "${Value}")
  set( ${Name} "${Value}" )
#  message("---" [${Name}] "  " [${Value}] )
endforeach()                                     

