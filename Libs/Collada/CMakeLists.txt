cmake_minimum_required (VERSION 3.0)
project( Collada )

set                   ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../../../Sources/CMake/Modules/" )
include               ( CMake-common )

find_package          ( QT5           REQUIRED )

include_directories( ${Qt5Core_INCLUDE_DIRS} )
include_directories( ${Qt5Gui_INCLUDE_DIRS} )
include_directories( ${Qt5Widgets_INCLUDE_DIRS} )
include_directories( ${Qt5Qml_INCLUDE_DIRS} )
include_directories( ${Qt5Quick_INCLUDE_DIRS} )   

include_directories   ( ${DAVA_INCLUDE_DIR} )
include_directories   ( ${DAVA_TOOLS_DIR} )
include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/FCollada )
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ColladaConverter/Collada15/External/Cg/include )
include_directories   ( ${DAVA_ROOT_DIR}/Tools/ResourceEditor/Classes )

define_source_folders          ( SRC_ROOT "." )
generate_source_groups_project ( )

add_library                    ( ${PROJECT_NAME} STATIC ${PROJECT_SOURCE_FILES} )
	