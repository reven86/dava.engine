cmake_minimum_required (VERSION 3.0)
project( bullet )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include               ( CMake-common )

include_directories   ( . )
include_directories   ( ${DAVA_INCLUDE_DIR} )
include_directories   ( ${DAVA_TOOLS_DIR} )
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

define_source_folders          ( SRC_ROOT "." )
generate_source_groups_project ( )

add_library                    ( ${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES} )
	