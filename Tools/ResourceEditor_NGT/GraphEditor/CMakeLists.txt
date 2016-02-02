cmake_minimum_required (VERSION 3.0)
project( GraphEditor )

find_package(NGT REQUIRED)

list(APPEND TARGET_LIBRARIES core_generic_plugin core_qt_common core_ui_framework)

include_directories   ( ${CMAKE_CURRENT_BINARY_DIR})
include_directories   ( "Classes" )
include_directories   (${ADDITIONAL_INCLUDES})

file(GLOB_RECURSE META_LIST "Classes/*.mpp")
file ( GLOB QML_LIST "DataQt/*.qml" )
file ( GLOB UI_LIST "DataQt/*.ui" )
file ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_wrap_ui ( UI_APP_HDRS  ${UI_LIST} )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

set ( Generated          ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source_folders          ( SRC_ROOT "Classes" "Classes/Metadata" GLOBAL_ERASE_FOLDER )
generate_source_groups_project ( SRC_ROOT "Classes" "Classes/Metadata" "DataQt" Generated)

set( ADDED_SRC            ${QML_LIST} ${UI_APP_HDRS} ${RCC_APP_SRCS} ${META_LIST})

append_deploy_dependency(${PROJECT_NAME})
setup_main_dynlib()

BW_GET_EXECUTABLE_DST_DIR(EXE_DIR)
BW_SET_BINARY_DIR(${PROJECT_NAME} "${EXE_DIR}/plugins")
if (MACOS)
  BW_COPY_PLUGIN_TO_BUNDLE(${PROJECT_NAME})
endif()

