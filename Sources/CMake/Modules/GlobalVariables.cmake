
set ( DAVA_LIBRARY                     "DavaFramework" )
set ( DAVA_ROOT_DIR                    "${CMAKE_CURRENT_LIST_DIR}/../../.." )
set ( DAVA_ENGINE_DIR                  "${CMAKE_CURRENT_LIST_DIR}/../../Engine" )
set ( DAVA_THIRD_PARTY_ROOT_PATH       "${CMAKE_CURRENT_LIST_DIR}/../../../Libs" )
set ( DAVA_THIRD_PARTY_INCLUDES_PATH   "${DAVA_THIRD_PARTY_ROOT_PATH}/include" )

if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/ios" ) 
    
else   ()
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib/win" ) 
    
endif  ()


set ( DAVA_INCLUDE_DIR            "${DAVA_ENGINE_DIR}" "${DAVA_THIRD_PARTY_INCLUDES_PATH}"  )


set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4-Windows"
     "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4-Windows/Magick++/lib" )

file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_PATH "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/ImageMagick-6.7.4-Windows/VisualMagick/lib/CORE_RL_*.lib" )


