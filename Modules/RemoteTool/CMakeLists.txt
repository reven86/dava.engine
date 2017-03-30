set(NAME_MODULE RemoteTool)

set(MODULE_TYPE STATIC)

set(HPP_FILES_RECURSE *.h *.hpp)
set(CPP_FILES_RECURSE *.cpp *.mm)

set(INCLUDES "${CMAKE_CURRENT_LIST_DIR}" "Sources" "Sources/RemoteTool" "Sources/RemoteTool/Private/CustomPlot")
set(INCLUDES_PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR} ${DAVA_INCLUDE_DIR})

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

set(DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set(DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

setup_main_module()
