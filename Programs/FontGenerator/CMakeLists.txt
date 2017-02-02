cmake_minimum_required( VERSION 3.0 )

project               ( FontGenerator )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include               ( CMake-common )

include_directories   (.)
include_directories   (${DAVA_ENGINE_DIR})
include_directories   (${DAVA_THIRD_PARTY_INCLUDES_PATH})

#add_definitions       ( -D_CRT_SECURE_NO_DEPRECATE )

define_source ( SOURCE "Classes" "ThirdParty" )

#set( APP_DATA             ${CMAKE_CURRENT_LIST_DIR}/Data )
#set( MACOS_PLIST          ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/Info.plist )
#set( MACOS_ICO            ${CMAKE_CURRENT_LIST_DIR}/MacOSSpecific/icon.icns  )
#set( MACOS_DYLIB          )
#set( ADDED_SRC            )
set( LIBRARIES             "${DAVA_THIRD_PARTY_ROOT_PATH}/freetype/lib/freetype246MT.lib" )
#set( ADDED_BINARY_DIR     )

setup_main_executable()

if (WIN32)
	set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE /NODEFAULTLIB:LIBCMT" )
endif ()

