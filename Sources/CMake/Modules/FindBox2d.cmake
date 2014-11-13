get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH CACHE)
add_subdirectory ("${CURRENT_DIR}/../../ThirdParty/Box2D" ${CMAKE_CURRENT_BINARY_DIR}/Box2D)

