cmake_minimum_required (VERSION 2.8.6)

project (DavaFramework)

set ( FRAMEWORK_DEFINITIONS 
#-D_DEBUG
#-DDAVA_DEBUG
-D_UNICODE 
-DUNICODE 
-D_CRT_SECURE_NO_DEPRECATE 
-D_CRT_SECURE_NO_DEPRECATE
-D_CRT_NON_CONFORMING_SWPRINTFS
-DCURL_STATICLIB
-DDAVA_FMOD
)

set ( LIBRARIE_TYPE STATIC )
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../CMake/Modules/" ) 
set ( THIRD_PARTY_INCLUDES_PATH   "${CMAKE_CURRENT_LIST_DIR}/../../Libs/include" )
set ( THIRD_PARTY_LIBRARIES_PATH  "${CMAKE_CURRENT_LIST_DIR}/../../Libs/lib" ) 
set ( LIST_SHARED_LIBRARIES )

find_package(ZLIB )
if( ZLIB_FOUND )
    list (APPEND THIRD_PARTY_INCLUDES_PATH  ${ZLIB_INCLUDE_DIR} )
    list (APPEND LIST_SHARED_LIBRARIES ${ZLIB_LIBRARY}     )
endif()

if( ANDROID )
    set ( LIBRARIE_TYPE SHARED )

    link_directories( "${CMAKE_CURRENT_LIST_DIR}/../../Libs/lib/android/armeabi-v7a" )
    list (APPEND LIST_SHARED_LIBRARIES
        "libxml_android.a"
        "libpng_android.a"
        "libfreetype_android.a"
        "libyaml_android.a"
        "libmongodb_android.a"
        "liblua_android.a"
        "libdxt_android.a"
        "libjpeg_android.a"
        "libcurl_android.a"
        "libssl_android.a"
        "libcrypto_android.a"
        "libbox2d.a"
        "libfmodex.so"
        "libfmodevent.so"
        "libfmodexL.so"
        "libfmodeventL.so"
        "libiconv_android.so"
        "libEGL.so"
        "libGLESv1_CM.so"
        "liblog.so"
        "libandroid.so"
        "-lGLESv2"
        "-lstdc++"
        )
        
    set ( FRAMEWORK_DEFINITIONS  ${FRAMEWORK_DEFINITIONS} -D___ANDROID__ -DDAVA_FMOD )
    set ( THIRD_PARTY_LIBRARIES_PATH  "${CMAKE_CURRENT_LIST_DIR}/../../Libs/lib/android/${ANDROID_NDK_ABI_NAME}" ) 
    set ( LIBRARY_OUTPUT_PATH    ${THIRD_PARTY_LIBRARIES_PATH} )
    
elseif( IOS ) 
    set ( CMAKE_C_FLAGS    "-mno-thumb"  )
    set ( CMAKE_CXX_FLAGS  "-mno-thumb -fvisibility=hidden" "libcd.lib" )
    
else()
    if (MSVC)
       set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
       set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
       add_definitions       ( -D_ITERATOR_DEBUG_LEVEL=0 )
    endif()

    file ( GLOB LIB_LIST "${THIRD_PARTY_LIBRARIES_PATH}/win/*.lib" )
    list ( APPEND LIST_SHARED_LIBRARIES ${LIB_LIST}  )
    list ( APPEND LIST_SHARED_LIBRARIES "opengl32.lib" )

endif()

include ( CMake-common )

# Define source 
define_source_folders  ()
generate_source_groups_project ()

include_directories   (.)
include_directories   ( ${THIRD_PARTY_INCLUDES_PATH} ) 
                                          
add_definitions       ( ${FRAMEWORK_DEFINITIONS} )

add_library           ( DavaFramework ${LIBRARIE_TYPE} ${PROJECT_SOURCE_FILES} )

target_link_libraries ( DavaFramework ${LIST_SHARED_LIBRARIES} )  
MERGE_STATIC_LIBRARIES( DavaFramework ALL ${LIB_LIST} )


#install(
#    TARGETS
#    DavaFramework
#    DESTINATION
#    ${THIRD_PARTY_LIBRARIES_PATH}/
#)

#install(
#    DIRECTORY
#    ${CMAKE_CURRENT_SOURCE_DIR}/
#    DESTINATION
#    ${THIRD_PARTY_INCLUDE_PATH}/DavaFramework
#    FILES_MATCHING
#    PATTERN
#    "*.h"
#)