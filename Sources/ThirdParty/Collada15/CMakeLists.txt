cmake_minimum_required (VERSION 2.8.6)

project          ( Collada15 )

set              ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../CMake/Modules/" ) 
include          ( CMake-common )

set              ( COLLADA_INCLUDES             "${CMAKE_CURRENT_LIST_DIR}/FCollada" )
set              ( LIBRARY_XML_INCLUDES         "${CMAKE_CURRENT_LIST_DIR}/FCollada/LibXML/include" )

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions( -D_ITERATOR_DEBUG_LEVEL=0 )
endif()


add_subdirectory ( "FCollada"                    ${PROJECT_BINARY_DIR}/FCollada)
add_subdirectory ( "FCollada/LibXML"             ${CMAKE_CURRENT_BINARY_DIR}/LibXML )
add_subdirectory ( "FCollada/FColladaTest"       ${CMAKE_CURRENT_BINARY_DIR}/FColladaTest )









  
