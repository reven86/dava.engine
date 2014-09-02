cmake_minimum_required(VERSION 2.6)

project(Box2D)


option(BOX2D_INSTALL "Install Box2D libs, includes, and CMake scripts" ON )
option(BOX2D_INSTALL_DOC "Install Box2D documentation" OFF)
option(BOX2D_BUILD_SHARED "Build Box2D shared libraries" OFF)
option(BOX2D_BUILD_STATIC "Build Box2D static libraries" ON)

set(BOX2D_VERSION 2.1.0)

# The Box2D library.
add_subdirectory(Box2D)

if(BOX2D_INSTALL_DOC)
  install(DIRECTORY Documentation DESTINATION share/doc/Box2D PATTERN ".svn" EXCLUDE)
endif(BOX2D_INSTALL_DOC)
                      
if (NOT BOX2D_SOURCE_NAME)
   set(BOX2D_SOURCE_NAME Box2d)
endif (NOT BOX2D_SOURCE_NAME)

set (BOX2D_INCLUDE_DIR "${PROJECT_SOURCE_DIR}" CACHE STRING "Box2D include directory" FORCE)
set (BOX2D_LIBRARY "BOX2D" CACHE STRING "Box2D library" FORCE)
set (BOX2D_LIBRARIES "${BOX2D_LIBRARY}" CACHE STRING "Box2D libraries" FORCE)
set (BOX2D_FOUND 1 CACHE STRING "Set to 1 if Box2D is found, 0 otherwise" FORCE)

include_directories ("." ${BOX2D_INCLUDE_DIR})
