cmake_minimum_required (VERSION 2.8.6)

project (DavaFramework)

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../CMake/Modules/" ) 

include ( CMake-common )

set ( FRAMEWORK_DEFINITIONS 
#-D_DEBUG
-DDAVA_DEBUG
-D_UNICODE 
-DUNICODE 
-D_CRT_SECURE_NO_DEPRECATE 
-D_CRT_SECURE_NO_DEPRECATE
-D_CRT_NON_CONFORMING_SWPRINTFS
-DCURL_STATICLIB
-DDAVA_FMOD
)

file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*" )
list ( APPEND LIST_SHARED_LIBRARIES ${LIB_LIST}  )

file ( GLOB LIB_LIST "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/*" )
list ( APPEND LIST_SHARED_LIBRARIES_DEBUG ${LIB_LIST}  )
         
if( ANDROID )
    set ( LIBRARY_TYPE SHARED )
    
    link_directories( "${DAVA_THIRD_PARTY_LIBRARIES_PATH}" )
    set ( LIST_LIBRARIES
        "libcrypto_android.a"
        "libEGL.so"
        "libGLESv1_CM.so"
        "liblog.so"
        "libandroid.so"
        "-lGLESv2"
        "-lstdc++" )
        
    list ( APPEND LIST_SHARED_LIBRARIES        ${LIST_LIBRARIES} )
    list ( APPEND LIST_SHARED_LIBRARIES_DEBUG  ${LIST_LIBRARIES} )

    set ( FRAMEWORK_DEFINITIONS  ${FRAMEWORK_DEFINITIONS} -D___ANDROID__ -DDAVA_FMOD )        
    set ( LIBRARY_OUTPUT_PATH    ${THIRD_PARTY_LIBRARIES_PATH} )
    
elseif( IOS ) 
    set ( LIBRARY_TYPE STATIC )
    
    set ( CMAKE_C_FLAGS    "-mno-thumb"  )
    set ( CMAKE_CXX_FLAGS  "-mno-thumb -fvisibility=hidden" "libcd.lib" )
    
else()
    set ( LIBRARY_TYPE STATIC ) 
    
    if ( MSVC )
       set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
       set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
       add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
       
       list ( APPEND LIST_SHARED_LIBRARIES        "opengl32.lib" )
       list ( APPEND LIST_SHARED_LIBRARIES_DEBUG  "opengl32.lib" )
       
    endif()

endif()

find_package(ZLIB )
if( ZLIB_FOUND )
    list (APPEND DAVA_THIRD_PARTY_INCLUDES_PATH ${ZLIB_INCLUDE_DIR} )
    list (APPEND LIST_SHARED_LIBRARIES          ${ZLIB_LIBRARY}     )
    list (APPEND LIST_SHARED_LIBRARIES_DEBUG    ${ZLIB_LIBRARY}     )
endif()


# Define source 
define_source_folders  ()
generate_source_groups_project ()

include_directories    (.)
include_directories    ( ${DAVA_INCLUDE_DIR} )
include_directories    ( ${ADD_ENGINE_INCLUDES_DIR} ) 
include_directories    ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} ) 
                                          
add_definitions        ( ${FRAMEWORK_DEFINITIONS} )

add_library            ( DavaFramework ${LIBRARY_TYPE} ${PROJECT_SOURCE_FILES} )

target_link_libraries  ( DavaFramework 
    debug   ${LIST_SHARED_LIBRARIES_DEBUG} 
    general ${LIST_SHARED_LIBRARIES} 
)  

#install(
#    TARGETS
#    DavaFramework
#    DESTINATION
#    ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )

#install(
#    DIRECTORY
#    ${CMAKE_CURRENT_SOURCE_DIR}/
#    DESTINATION
#    ${THIRD_PARTY_INCLUDE_PATH}/DavaFramework
#    FILES_MATCHING
#    PATTERN
#    "*.h")
