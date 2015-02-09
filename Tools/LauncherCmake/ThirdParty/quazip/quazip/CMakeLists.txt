# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIRS}
)

file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

qt5_wrap_cpp(MOC_SRCS ${PUBLIC_HEADERS})
set(SRCS ${SRCS} ${MOC_SRCS})

add_library(quazip STATIC ${SRCS})
set_target_properties(quazip PROPERTIES VERSION 1.0.0 SOVERSION 1)
# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(quazip ${QT_QTMAIN_LIBRARY} ${QT_QTCORE_LIBRARY} ${ZLIB_LIBRARIES})

