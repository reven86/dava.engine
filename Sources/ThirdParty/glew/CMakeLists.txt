cmake_minimum_required (VERSION 2.6)

project (GLEW)

add_definitions ( -DGLEW_STATIC )
#add_definitions ( -DGLEW_BUILD )

if (NOT GLEW_SOURCE_NAME)
   set(GLEW_SOURCE_NAME glew-1.7.0)
endif (NOT GLEW_SOURCE_NAME)

set (GLEW_SRC_FOLDER "${PROJECT_SOURCE_DIR}/${GLEW_SOURCE_NAME}")
set (GLEW_INCLUDE_DIR "${GLEW_SRC_FOLDER}/include" CACHE STRING "GLEW include directory" FORCE)
set (GLEW_INCLUDE_DIRS "${GLEW_INCLUDE_DIR}" CACHE STRING "GLEW include directory" FORCE)
set (GLEW_LIBRARY "GLEW" CACHE STRING "GLEW library" FORCE)
set (GLEW_LIBRARIES "${GLEW_LIBRARY}" CACHE STRING "GLEW libraries" FORCE)
set (GLEW_FOUND 1 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise" FORCE)

set (GLEW_src
   ${GLEW_SRC_FOLDER}/src/glew.c
)

include_directories ("." ${GLEW_INCLUDE_DIRS})

add_library (${GLEW_LIBRARIES} STATIC ${GLEW_src})
#add_library (${GLEW_LIBRARIES} SHARED ${GLEW_src})

install(
    TARGETS
    ${GLEW_LIBRARIES}
    DESTINATION
    ${THIRD_PARTY_LIBS_PATH}/
)

install(
    DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/glew-1.7.0/include/GL/
    DESTINATION
    ${THIRD_PARTY_INCLUDE_PATH}/GL
    FILES_MATCHING
    PATTERN
    "*.h"
)