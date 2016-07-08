cmake_minimum_required(VERSION 3.4)
project(sqlite3)

include_directories(.)

set(SOURCE_FILES sqlite3.c)

add_library(sqlite3 STATIC ${SOURCE_FILES})

if(WIN32)
  set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd" ) 
  set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT" ) 
elseif(APPLE)
  if(IOS)
    set( CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY iPhone/iPad )
    set( CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 7.0)
  else()
    set( CMAKE_OSX_DEPLOYMENT_TARGET 10.8 )
    set( CMAKE_OSX_ARCHITECTURES x86_64;i386 )
  endif()
endif()
