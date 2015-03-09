cmake_minimum_required (VERSION 2.8)

project( lua )
 
include_directories (.)

file ( GLOB LIB_SOURCES "*.h" "*.c" "*.cpp" )

add_library( ${PROJECT_NAME} STATIC  ${LIB_SOURCES} )


