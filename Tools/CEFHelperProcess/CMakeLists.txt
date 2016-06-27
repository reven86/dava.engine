cmake_minimum_required (VERSION 3.0)
project ( CEFHelperProcess )

string ( FIND ${CMAKE_GENERATOR} "Win64" X64_PROJECT )
if( ${X64_PROJECT} EQUAL -1 )
    set ( ARCH "x86" )
else ()
    set ( ARCH "x64" )
endif ()

set ( DAVA_ENGINE_LIBS "${CMAKE_CURRENT_LIST_DIR}/../../Libs" )
set ( DAVA_ENGINE_INCLUDES "${CMAKE_CURRENT_LIST_DIR}/../../Sources/Internal" )
set ( CEF_LIBS "${DAVA_ENGINE_LIBS}/lib_CMake/win/${ARCH}/Release" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd" ) 
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT" ) 

include_directories ( "${DAVA_ENGINE_INCLUDES}" 
                      "${DAVA_ENGINE_LIBS}/include" 
                      "${DAVA_ENGINE_LIBS}/include/cef" )

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    "${CEF_LIBS}/libcef.lib"
    "${CEF_LIBS}/libcef_dll_wrapper.lib"
)
