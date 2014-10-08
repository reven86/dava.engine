
if ( GLOBAL_VAR_FOUND )
    return ()
endif ()
set ( GLOBAL_VAR_FOUND 1 )


set ( DAVA_LIBRARY                     "DavaFramework" )
set ( DAVA_ROOT_DIR                    "${CMAKE_CURRENT_LIST_DIR}/../../.." )
set ( DAVA_ENGINE_DIR                  "${CMAKE_CURRENT_LIST_DIR}/../../Engine" )
set ( DAVA_THIRD_PARTY_ROOT_PATH       "${CMAKE_CURRENT_LIST_DIR}/../../../Libs" )
set ( DAVA_THIRD_PARTY_INCLUDES_PATH   "${DAVA_THIRD_PARTY_ROOT_PATH}/include" )

GET_FILENAME_COMPONENT( DAVA_ROOT_DIR              ${DAVA_ROOT_DIR}              ABSOLUTE)
GET_FILENAME_COMPONENT( DAVA_ENGINE_DIR            ${DAVA_ENGINE_DIR}            ABSOLUTE)
GET_FILENAME_COMPONENT( DAVA_THIRD_PARTY_ROOT_PATH ${DAVA_THIRD_PARTY_ROOT_PATH} ABSOLUTE)


if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/ios" ) 
    
elseif ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/mac" ) 

else   ()
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/win" ) 
    
endif  ()

set ( DAVA_INCLUDE_DIR ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

if( APPLE ) 

	set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4/include/ImageMagick"
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4/delegates/include" 
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4/" 
	     )

	set ( DAVA_IMAGE_MAGICK_LIBRARIES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4" )  

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE  ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/lib/*.a ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/delegates/lib/*.a )

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE}" )

else()

	set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4-Windows"
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4-Windows/Magick++/lib" )

	set ( DAVA_IMAGE_MAGICK_LIBRARIES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4-Windows/VisualMagick/lib" )  

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE  "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )
	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_DB_*.lib" )

endif()





