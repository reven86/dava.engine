#cmake_minimum_required (VERSION 2.8.6)
cmake_minimum_required (VERSION 3.0)

project (DavaFramework)

set ( INCLUDES_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Libs/include" )

if( ANDROID )
    set ( LIBRARIE_TYPE SHARED )

    link_directories( "${CMAKE_CURRENT_LIST_DIR}/../../Libs/lib/android/armeabi-v7a" )

    set ( LIBRARIES_DIR 
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

elseif( IOS ) 
    set( CMAKE_C_FLAGS    "-mno-thumb"  )
    set( CMAKE_CXX_FLAGS  "-mno-thumb -fvisibility=hidden"  )
else()
    set ( LIBRARIES_DIR  "${CMAKE_CURRENT_LIST_DIR}/../../Libs/lib")
    set ( LIBRARIE_TYPE STATIC )
endif()

set ( LIBRARY_OUTPUT_PATH ${LIBRARIES_DIR} PARENT_SCOPE )

# Define source files
#define_source_files (PARENT_SCOPE)
FILE(GLOB SOURCE_FOLDERS "*" )
FOREACH(FOLDER_ITEM ${SOURCE_FOLDERS})
    IF( IS_DIRECTORY "${FOLDER_ITEM}" )
        FILE(GLOB FIND_CMAKELIST "${FOLDER_ITEM}/CMakeLists.txt")
	IF( FIND_CMAKELIST )
            get_filename_component ( FOLDER_NAME ${FOLDER_ITEM} NAME ) 
            set ( SOURCES ${SOURCES} ${FOLDER_NAME} )
	ENDIF()
    ENDIF()
ENDFOREACH()

foreach (SOURCE ${SOURCES})
    add_subdirectory (${SOURCE})
    source_group ("${SOURCE}" FILES ${${SOURCE}_H_FILES} FILES ${${SOURCE}_CPP_FILES} )
    list (APPEND ENGINE_SOURCE_FILES ${${SOURCE}_CPP_FILES} ${${SOURCE}_H_FILES})
    list (APPEND ENGINE_INCLUDE_DIRS_ONLY ${SOURCE})
endforeach ()

define_source_files (PARENT_SCOPE)
list (APPEND ENGINE_SOURCE_FILES ${CPP_FILES} ${H_FILES})

find_package(ZLIB REQUIRED)

include_directories   (.)
include_directories   ( ${ZLIB_INCLUDE_DIR} )
include_directories   ( ${INCLUDES_DIR} ) 
                                             
add_definitions ( ${FRAMEWORK_DEFINITIONS} )
add_library ( DavaFramework ${LIBRARIE_TYPE} ${ENGINE_SOURCE_FILES} )

target_link_libraries (DavaFramework ${LIBRARIES_DIR} ${ZLIB_LIBRARY} )  

install(
    TARGETS
    DavaFramework
    DESTINATION
    ${THIRD_PARTY_LIBS_PATH}/
)

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION
    ${THIRD_PARTY_INCLUDE_PATH}/DavaFramework
    FILES_MATCHING
    PATTERN
    "*.h"
)