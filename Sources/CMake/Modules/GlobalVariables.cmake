
if ( GLOBAL_VAR_FOUND )
    return ()
endif ()
set ( GLOBAL_VAR_FOUND 1 )

if( APPLE AND NOT IOS )
	set ( MACOS 1 )
endif ()

set ( DAVA_LIBRARY                     "DavaFramework" )
set ( DAVA_ROOT_DIR                    "${CMAKE_CURRENT_LIST_DIR}/../../.." )

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

#ImageMagick
if( MACOS ) 

        set( IMAGE_MAGICK ImageMagick-6.8.9-Mac )

	set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}/include/ImageMagick"
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}/delegates/include" 
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}/" 
	     )

	set ( DAVA_IMAGE_MAGICK_LIBRARIES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}" )  

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE  ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/lib/*.a ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/delegates/lib/*.a )

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE}" )

elseif( IOS ) 

elseif( WIN32 )

        set( IMAGE_MAGICK ImageMagick-6.8.9-Windows )

	set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}"
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}/Magick++/lib" )

	set ( DAVA_IMAGE_MAGICK_LIBRARIES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/${IMAGE_MAGICK}/VisualMagick/lib" )  

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE  "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )
	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_DB_*.lib" )

endif()





