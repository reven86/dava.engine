project( NgtTools )

if (WIN32)
    add_definitions (-D_CRT_SECURE_NO_DEPRECATE)
elseif (MACOS AND NOT ANDROID)
    add_definitions (-DDARWIN_NO_CARBON)
endif()

include               ( CMake-common )
include               ( NGTMacro )

get_ngt_modules(NGT_LIBS NGT_PLUGINS QT_COMPONENTS)

include_directories ( ${DAVA_ROOT_DIR}/Sources/Internal)
include_directories ( ${DAVA_ROOT_DIR}/Sources/Tools)

find_package ( QT5 REQUIRED COMPONENTS ${QT_COMPONENTS} )
find_package ( NGT REQUIRED )

ASSERT(NGT_OUTPUT_DIR "Please call \"configure_ngt\" function from NGTMarco module before find NgtTools")

 if (NOT DEPLOY_DIR)
      BW_SET_USE_CONFIGURATION_NAME(1)
endif()
BW_SET_TEST_DST_DIR(${NGT_OUTPUT_DIR})
BW_SET_ASSET_PIPELINE_DST_DIR(${NGT_OUTPUT_DIR})
BW_SET_EXECUTABLE_DST_DIR(${NGT_OUTPUT_DIR})

set( MOC_APP_SRCS )
set( UI_APP_HDRS  )
set( RCC_APP_SRCS )

file              ( GLOB_RECURSE UI_LIST "*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB_RECURSE RCC_LIST "*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

file( GLOB_RECURSE WIDGETS_SRC "*.h"  "*.cpp" )
set( ADDED_SRC ${UI_APP_HDRS} ${RCC_APP_SRCS} ${WIDGETS_SRC} )
set( Generated ${UI_APP_HDRS} ${RCC_APP_SRCS} )

generated_unity_sources        ( ADDED_SRC )
generate_source_groups_project ( GROUP_FOLDERS Generated )

# append NGT targets to solution
foreach(NGT_LIB ${NGT_LIBS})
    append_ngt_lib(${NGT_LIB})
endforeach()

foreach(NGT_PLUGIN ${NGT_PLUGINS})
    append_ngt_plugin(${NGT_PLUGIN})
endforeach()

list(APPEND TARGET_LIBRARIES ${NGT_LIBS})
add_library                    ( ${PROJECT_NAME} STATIC ${ADDED_SRC} )
target_link_libraries  ( ${PROJECT_NAME} ${TARGET_LIBRARIES} )
