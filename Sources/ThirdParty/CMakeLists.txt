cmake_minimum_required (VERSION 2.8.6)
project (ThirdParty)

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../CMake/Modules/" ) 
include ( CMake-common )


add_subdirectory ("Box2D" ${PROJECT_BINARY_DIR}/Box2D)
add_subdirectory ("glew" ${PROJECT_BINARY_DIR}/glew )




  
