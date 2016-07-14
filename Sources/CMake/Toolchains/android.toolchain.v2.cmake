cmake_minimum_required( VERSION 2.6.3 )
find_package( PythonInterp )

set     ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../Modules/" ) 
include ( GlobalVariables )

if ( NOT EXISTS "${ANDROID_NDK}" )
    message ( FATAL_ERROR "Android NDK is not found: ${ANDROID_NDK}" )
endif ()

# Standard settings
set ( CMAKE_SYSTEM_NAME Android )
set ( CMAKE_SYSTEM_VERSION 1 )

if ( WIN32 )
    set ( HOST_SYSTEM "windows-x86_64" )
    set ( TOOL_OS_SUFFIX ".exe" )
endif ()

STRING( REGEX REPLACE "\\\\" "/" ANDROID_NDK ${ANDROID_NDK} )
set ( CLANG_DIR "${ANDROID_NDK}/toolchains/llvm/prebuilt/${HOST_SYSTEM}/bin" )
set ( CLANG_C_PATH "${CLANG_DIR}/clang" )
set ( CLANG_CXX_PATH "${CLANG_DIR}/clang++" )

if ( WIN32 )
    set ( CLANG_C_PATH "${CLANG_C_PATH}.exe" )
    set ( CLANG_CXX_PATH "${CLANG_CXX_PATH}.exe" )
endif ()

if ( NOT EXISTS "${CLANG_C_PATH}" OR NOT EXISTS "${CLANG_CXX_PATH}" )
    message ( FATAL_ERROR "Clang is not found" )
endif ()

# Set clang compiler
include (CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER   ( "${CLANG_C_PATH}"   clang   )
CMAKE_FORCE_CXX_COMPILER ( "${CLANG_CXX_PATH}" clang++ )

# Skip the platform compiler checks for cross compiling
set ( CMAKE_CXX_COMPILER_WORKS TRUE )
set ( CMAKE_C_COMPILER_WORKS TRUE )

# Set target ABI options
if( ANDROID_ABI STREQUAL "x86" )
    set( X86 true )
    set( ANDROID_NDK_ABI_NAME "x86" )
    set( ANDROID_ARCH_NAME "x86" )
    set( ANDROID_TARGET "i686-linux-android" )
    set( ANDROID_LLVM_TRIPLE "i686-none-linux-android" )
    set( CMAKE_SYSTEM_PROCESSOR "i686" )
    set( NEON true )
    set( ANDROID_TOOLCHAIN_NAME "x86-4.9" )
elseif( ANDROID_ABI STREQUAL "armeabi-v7a" )
    set( ARMEABI_V7A true )
    set( ANDROID_NDK_ABI_NAME "armeabi-v7a" )
    set( ANDROID_ARCH_NAME "arm" )
    set( ANDROID_TARGET "arm-linux-androideabi" )
    set( ANDROID_LLVM_TRIPLE "armv7-none-linux-androideabi" )
    set( CMAKE_SYSTEM_PROCESSOR "armv7-a" )
    set( NEON true )
    set( ANDROID_TOOLCHAIN_NAME "arm-linux-androideabi-4.9" )
else()
    message( SEND_ERROR "Unknown ANDROID_ABI=\"${ANDROID_ABI}\" is specified." )
endif()

set( ANDROID_SYSROOT "${ANDROID_NDK}/platforms/android-${ANDROID_NATIVE_API_LEVEL}/arch-${ANDROID_ARCH_NAME}" )
set( ANDROID_GCC_TOOLCHAIN_ROOT "${ANDROID_NDK}/toolchains/${ANDROID_TOOLCHAIN_NAME}/prebuilt/${HOST_SYSTEM}" )
set( ANDROID_TOOLCHAIN_ROOT "${ANDROID_GCC_TOOLCHAIN_ROOT}/${ANDROID_TARGET}" )

set ( ANDROID_STL_PATH "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++" )
set ( ANDROID_STL_LIB_DIR "${ANDROID_STL_PATH}/libs/${ANDROID_NDK_ABI_NAME}" )
set ( ANDROID_STL_SO_NAME "libc++_shared.so" )
set ( ANDROID_STL_SO_PATH "${ANDROID_STL_LIB_DIR}/${ANDROID_STL_SO_NAME}" )

# Include dirs
set( ANDROID_INCLUDES "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libcxx/include"
                      "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++abi/libcxxabi/include"
                      "${ANDROID_NDK}/sources/android/support/include"
                      "${ANDROID_SYSROOT}/usr/include" )
                      
# Libraries
set ( ANDROID_LINK_LIBRARIES "-lc++abi -landroid_support -latomic -lm \"${ANDROID_STL_SO_PATH}\"" )

# Unwind is presented for arm only
if ( ARMEABI_V7A )
    set ( ANDROID_LINK_LIBRARIES "${ANDROID_LINK_LIBRARIES} -lunwind" )
endif ()
                      
# Install libs search directories
include_directories( SYSTEM  ${ANDROID_INCLUDES} )
link_directories( "${ANDROID_STL_LIB_DIR}" )

# Configure compiler and other tools
set( CMAKE_C_COMPILER_ID   Clang )
set( CMAKE_CXX_COMPILER_ID Clang )
set( CMAKE_C_PLATFORM_ID   Linux )
set( CMAKE_CXX_PLATFORM_ID Linux )
set( CMAKE_C_COMPILER_VERSION   "3.8" )
set( CMAKE_CXX_COMPILER_VERSION "3.8" )

set( CMAKE_CXX_COMPILER_ABI ELF )
set( CMAKE_CXX_SOURCE_FILE_EXTENSIONS cc cp cxx cpp CPP c++ C )
set( CMAKE_C_COMPILER_ABI   ELF )
set( CMAKE_CXX_COMPILER_ABI ELF )
set( CMAKE_CXX_HAS_ISYSROOT 1 )

if( X86_64 OR MIPS64 OR ARM64_V8A )
    set( CMAKE_C_SIZEOF_DATA_PTR 8 )
else()
    set( CMAKE_C_SIZEOF_DATA_PTR 4 )
endif()
set( CMAKE_CXX_SIZEOF_DATA_PTR ${CMAKE_C_SIZEOF_DATA_PTR} )

set( CMAKE_ASM_COMPILER "${ANDROID_TOOLCHAIN_ROOT}/bin/as${TOOL_OS_SUFFIX}"      CACHE PATH "assembler" )
set( CMAKE_STRIP        "${ANDROID_TOOLCHAIN_ROOT}/bin/strip${TOOL_OS_SUFFIX}"   CACHE PATH "strip" )
set( CMAKE_AR           "${ANDROID_TOOLCHAIN_ROOT}/bin/ar${TOOL_OS_SUFFIX}"      CACHE PATH "archive" )
set( CMAKE_LINKER       "${ANDROID_TOOLCHAIN_ROOT}/bin/ld${TOOL_OS_SUFFIX}"      CACHE PATH "linker" )
set( CMAKE_NM           "${ANDROID_TOOLCHAIN_ROOT}/bin/nm${TOOL_OS_SUFFIX}"      CACHE PATH "nm" )
set( CMAKE_OBJCOPY      "${ANDROID_TOOLCHAIN_ROOT}/bin/objcopy${TOOL_OS_SUFFIX}" CACHE PATH "objcopy" )
set( CMAKE_OBJDUMP      "${ANDROID_TOOLCHAIN_ROOT}/bin/objdump${TOOL_OS_SUFFIX}" CACHE PATH "objdump" )
set( CMAKE_RANLIB       "${ANDROID_TOOLCHAIN_ROOT}/bin/ranlib${TOOL_OS_SUFFIX}"  CACHE PATH "ranlib" )

# Reset of Android definition
remove_definitions( -DANDROID )
add_definitions( -DANDROID )

# Toolchain and target compiler flags
set( ANDROID_TOOLCHAIN_OPTION "-gcc-toolchain ${ANDROID_GCC_TOOLCHAIN_ROOT} -target ${ANDROID_LLVM_TRIPLE}" ) 

# Compiler flags
set( ANDROID_CXX_FLAGS 
"${ANDROID_TOOLCHAIN_OPTION} \
--sysroot=${ANDROID_SYSROOT} \
-ffunction-sections \
-funwind-tables \
-fstack-protector-strong \
-Wno-invalid-command-line-argument \
-Wno-unused-command-line-argument \
-no-canonical-prefixes \
-frtti -fexceptions -O2 -DNDEBUG" )
                       
set( ANDROID_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" )
set( ANDROID_CXX_FLAGS_DEBUG   "-O0 -UNDEBUG -fno-limit-debug-info -DNDK_DEBUG=1 -g" )
                       
# Linker flags
set ( ANDROID_LINKER_FLAGS "${ANDROID_TOOLCHAIN_OPTION} -no-canonical-prefixes" )
set ( ANDROID_EXE_LINKER_FLAGS "${ANDROID_EXE_LINKER_FLAGS} -fPIE -pie" )
                           
if ( X86 )
    set( ANDROID_CXX_FLAGS "${ANDROID_CXX_FLAGS} -fPIC" )
elseif ( ARMEABI_V7A )
    set( ANDROID_CXX_FLAGS "${ANDROID_CXX_FLAGS} -fpic -march=armv7-a -mfloat-abi=softfp -mfpu=neon" )
    set( ANDROID_CXX_FLAGS_RELEASE "${ANDROID_CXX_FLAGS_RELEASE} -mthumb" )
    set( ANDROID_CXX_FLAGS_DEBUG   "${ANDROID_CXX_FLAGS_DEBUG} -marm" )
    set( ANDROID_LINKER_FLAGS "${ANDROID_LINKER_FLAGS} -Wl,--fix-cortex-a8" )
else ()
    message( SEND_ERROR "Unknown ANDROID_ABI=\"${ANDROID_ABI}\" is specified." )
endif ()

set( CMAKE_CXX_FLAGS         "${ANDROID_CXX_FLAGS}"         CACHE STRING "c++ flags" )   
set( CMAKE_CXX_FLAGS_RELEASE "${ANDROID_CXX_FLAGS_RELEASE}" CACHE STRING "c++d flags" )
set( CMAKE_CXX_FLAGS_DEBUG   "${ANDROID_CXX_FLAGS_DEBUG}"   CACHE STRING "c++r flags" )

set( CMAKE_C_FLAGS         "${CMAKE_CXX_FLAGS}"         CACHE STRING "c flags" )
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "cr flags" )
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}"   CACHE STRING "cd flags" )

set( CMAKE_LINKER_FLAGS     "${ANDROID_LINKER_FLAGS}"   CACHE STRING "linker flags" )

set( CMAKE_SHARED_LINKER_FLAGS "${ANDROID_LINKER_FLAGS}" CACHE STRING "shared linker flags" )
set( CMAKE_MODULE_LINKER_FLAGS "${ANDROID_LINKER_FLAGS}" CACHE STRING "module linker flags" )
set( CMAKE_EXE_LINKER_FLAGS    "${ANDROID_LINKER_FLAGS} ${ANDROID_EXE_LINKER_FLAGS}" CACHE STRING "executable linker flags" )

# set these global flags for cmake client scripts to change behavior
set( ANDROID True )
set( BUILD_ANDROID True )

# where is the target environment
set( CMAKE_FIND_ROOT_PATH "${ANDROID_TOOLCHAIN_ROOT}/bin" "${ANDROID_TOOLCHAIN_ROOT}/${ANDROID_TOOLCHAIN_MACHINE_NAME}" "${ANDROID_SYSROOT}" "${CMAKE_INSTALL_PREFIX}" "${CMAKE_INSTALL_PREFIX}/share" )

# only search for libraries and includes in the ndk toolchain
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )

# PIC is necesary for Android 6 and good practice in common
set( CMAKE_POSITION_INDEPENDENT_CODE TRUE )

# Set make rules for C++ objects
set( CMAKE_CXX_CREATE_SHARED_LIBRARY 
"<CMAKE_C_COMPILER> \
<CMAKE_SHARED_LIBRARY_CXX_FLAGS> \
<LANGUAGE_COMPILE_FLAGS> \
<LINK_FLAGS> \
<CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> \
<CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME> \
-o <TARGET> <OBJECTS> <LINK_LIBRARIES>" )
                                     
set( CMAKE_CXX_CREATE_SHARED_MODULE 
"<CMAKE_C_COMPILER> \
<CMAKE_SHARED_LIBRARY_CXX_FLAGS> \
<LANGUAGE_COMPILE_FLAGS> \
<LINK_FLAGS> \
<CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> \
<CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME> \
-o <TARGET> <OBJECTS> <LINK_LIBRARIES>" )
                                      
set( CMAKE_CXX_LINK_EXECUTABLE 
"<CMAKE_C_COMPILER> \
<FLAGS> \
<CMAKE_CXX_LINK_FLAGS> \
<LINK_FLAGS> \
<OBJECTS> \
-o <TARGET> <LINK_LIBRARIES>" )

# Set make rules for C objects
set( CMAKE_C_CREATE_SHARED_LIBRARY 
"<CMAKE_C_COMPILER> \
<CMAKE_SHARED_LIBRARY_C_FLAGS> \
<LANGUAGE_COMPILE_FLAGS> \
<LINK_FLAGS> \
<CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> \
<CMAKE_SHARED_LIBRARY_SONAME_C_FLAG><TARGET_SONAME> \
-o <TARGET> <OBJECTS> <LINK_LIBRARIES>" )

set( CMAKE_C_CREATE_SHARED_MODULE
"<CMAKE_C_COMPILER> \
<CMAKE_SHARED_LIBRARY_C_FLAGS> \
<LANGUAGE_COMPILE_FLAGS> \
<LINK_FLAGS> \
<CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> \
<CMAKE_SHARED_LIBRARY_SONAME_C_FLAG><TARGET_SONAME> \
-o <TARGET> <OBJECTS> <LINK_LIBRARIES>" )

set( CMAKE_C_LINK_EXECUTABLE
"<CMAKE_C_COMPILER> \
<FLAGS> \
<CMAKE_C_LINK_FLAGS> \
<LINK_FLAGS> \
<OBJECTS> -o <TARGET> <LINK_LIBRARIES>" )

# Link additional libraries
set ( CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} ${ANDROID_LINK_LIBRARIES}" )
set ( CMAKE_CXX_CREATE_SHARED_MODULE  "${CMAKE_CXX_CREATE_SHARED_MODULE} ${ANDROID_LINK_LIBRARIES}" )
set ( CMAKE_CXX_LINK_EXECUTABLE       "${CMAKE_CXX_LINK_EXECUTABLE} ${ANDROID_LINK_LIBRARIES}" )
